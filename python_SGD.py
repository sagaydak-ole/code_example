import numpy as np
from time import time
from oracles import BinaryLogistic
from oracles import MulticlassLogistic
from scipy.special import expit
from scipy.sparse import csr_matrix


class GDClassifier:
    """
    Реализация метода градиентного спуска для произвольного
    оракула, соответствующего спецификации оракулов из модуля oracles.py
    """
    def __init__(self, loss_function, step_alpha=1, step_beta=0,
                 tolerance=1e-5, max_iter=1000, **kwargs):
        """
        loss_function - строка, отвечающая за функцию потерь классификатора.
        Может принимать значения:
        - 'binary_logistic' - бинарная логистическая регрессия
        - 'multinomial_logistic' - многоклассовая логистическая регрессия

        step_alpha - float, параметр выбора шага из текста задания

        step_beta- float, параметр выбора шага из текста задания

        tolerance - точность, по достижении которой, необходимо прекратить оптимизацию.
        Необходимо использовать критерий выхода по модулю разности соседних значений функции:
        если (f(x_{k+1}) - f(x_{k})) < tolerance: то выход

        max_iter - максимальное число итераций

        **kwargs - аргументы, необходимые для инициализации
        """
        self.loss_function = loss_function
        self.clf = None
        if(self.loss_function == 'binary_logistic'):
            self.clf = BinaryLogistic(**kwargs)
        else:
            self.clf = MulticlassLogistic(**kwargs)
        self.step_alpha = step_alpha
        self.step_beta = step_beta
        self.tolerance = tolerance
        self.max_iter = max_iter

    def fit(self, X, y, w_0=None, trace=True, X_test=None, y_test=None):
        """
        Обучение метода по выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        y - одномерный numpy array

        w_0 - начальное приближение в методе

        trace - переменная типа bool

        Если trace = True, то метод должен вернуть словарь history, содержащий информацию
        о поведении метода. Длина словаря history = количество итераций + 1 (начальное приближение)

        history['time']: list of floats, содержит интервалы времени между двумя итерациями метода
        history['func']: list of floats, содержит значения функции на каждой итерации
        (0 для самой первой точки)
        """
        history = {'time': [], 'func': [], 'accuracy': []}
        self.w = w_0
        if(X_test is None):
            X_test = X
        if(y_test is None):
            y_test = y
        if(self.w is None):
            if(self.loss_function == 'binary_logistic'):
                self.w = np.zeros(X.shape[1])
            else:
                self.w = np.zeros((self.clf.class_number, X.shape[1]))
        history['func'].append(self.get_objective(X, y))
        history['time'].append(0.)
        history['accuracy'].append(np.mean(self.predict(X_test) == y_test))
        for i in range(self.max_iter):
            start = time()
            self.w -= (self.step_alpha / ((i + 1) ** self.step_beta)) * self.get_gradient(X, y)
            end = time()
            history['func'].append(self.get_objective(X, y))
            history['time'].append(end - start)
            history['accuracy'].append(np.mean(self.predict(X_test) == y_test))
            if(abs(history['func'][-1] - history['func'][-2]) < self.tolerance):
                break
        if(trace):
            return history

    def predict(self, X):
        """
        Получение меток ответов на выборке X

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        return: одномерный numpy array с предсказаниями
        """
        probs = self.predict_proba(X)
        res = np.argmax(probs, axis=1)
        if(self.loss_function == 'binary_logistic'):
            return 2 * res - 1
        else:
            return res

    def predict_proba(self, X):
        """
        Получение вероятностей принадлежности X к классу k

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        return: двумерной numpy array, [i, k] значение соответветствует вероятности
        принадлежности i-го объекта к классу k
        """
        if(self.loss_function == 'binary_logistic'):
            probs = expit(X.dot(self.w.T).ravel())
            return np.vstack([1 - probs, probs]).T
        else:
            probs = X.dot(self.w.T)
            probs = np.exp(probs - probs.max(axis=1)[:, np.newaxis])
            probs /= probs.sum(axis=1)[:, np.newaxis]
            return probs

    def get_objective(self, X, y):
        """
        Получение значения целевой функции на выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array
        y - одномерный numpy array

        return: float
        """
        return self.clf.func(X, y, self.w)

    def get_gradient(self, X, y):
        """
        Получение значения градиента функции на выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array
        y - одномерный numpy array

        return: numpy array, размерность зависит от задачи
        """
        return self.clf.grad(X, y, self.w)

    def get_weights(self):
        """
        Получение значения весов функционала
        """
        return self.w


class SGDClassifier(GDClassifier):
    """
    Реализация метода стохастического градиентного спуска для произвольного
    оракула, соответствующего спецификации оракулов из модуля oracles.py
    """

    def __init__(self, loss_function, batch_size, step_alpha=1, step_beta=0,
                 tolerance=1e-5, max_iter=1000, random_seed=153, **kwargs):
        """
        loss_function - строка, отвечающая за функцию потерь классификатора.
        Может принимать значения:
        - 'binary_logistic' - бинарная логистическая регрессия
        - 'multinomial_logistic' - многоклассовая логистическая регрессия

        batch_size - размер подвыборки, по которой считается градиент

        step_alpha - float, параметр выбора шага из текста задания

        step_beta- float, параметр выбора шага из текста задания

        tolerance - точность, по достижении которой, необходимо прекратить оптимизацию
        Необходимо использовать критерий выхода по модулю разности соседних значений функции:
        если (f(x_{k+1}) - f(x_{k})) < tolerance: то выход


        max_iter - максимальное число итераций

        random_seed - в начале метода fit необходимо вызвать np.random.seed(random_seed).
        Этот параметр нужен для воспроизводимости результатов на разных машинах.

        **kwargs - аргументы, необходимые для инициализации
        """
        self.loss_function = loss_function
        self.clf = None
        if(self.loss_function == 'binary_logistic'):
            self.clf = BinaryLogistic(**kwargs)
        else:
            self.clf = MulticlassLogistic(**kwargs)
        self.step_alpha = step_alpha
        self.step_beta = step_beta
        self.tolerance = tolerance
        self.max_iter = max_iter
        self.batch_size = batch_size
        self.random_seed = random_seed

    def fit(self, X, y, w_0=None, trace=True, log_freq=1, X_test=None, y_test=None):
        """
        Обучение метода по выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        y - одномерный numpy array

        w_0 - начальное приближение в методе

        Если trace = True, то метод должен вернуть словарь history, содержащий информацию
        о поведении метода. Если обновлять history после каждой итерации, метод перестанет
        превосходить в скорости метод GD. Поэтому, необходимо обновлять историю метода лишь
        после некоторого числа обработанных объектов в зависимости от приближённого номера эпохи.
        Приближённый номер эпохи:
            {количество объектов, обработанных методом SGD} / {количество объектов в выборке}

        log_freq - float от 0 до 1, параметр, отвечающий за частоту обновления.
        Обновление должно проиходить каждый раз, когда разница между двумя значениями
        приближённого номера эпохи будет превосходить log_freq.

        history['epoch_num']: list of floats, в каждом элементе списка будет записан
        приближённый номер эпохи:
        history['time']: list of floats, содержит интервалы времени между двумя соседними замерами
        history['func']: list of floats, содержит значения функции
        после текущего приближённого номера эпохи
        history['weights_diff']: list of floats, содержит квадрат
        нормы разности векторов весов с соседних замеров
        (0 для самой первой точки)
        """
        # choose = 0
        # change = 0
        # ecalc = 0
        # ecomp = 0
        np.random.seed(self.random_seed)
        history = {'time': [], 'func': [], 'epoch_num': [], 'weights_diff': [], 'accuracy': []}
        self.w = w_0
        if(X_test is None):
            X_test = X
        if(y_test is None):
            y_test = y
        if(self.w is None):
            if(self.loss_function == 'binary_logistic'):
                self.w = np.zeros(X.shape[1])
            else:
                self.w = np.zeros((self.clf.class_number, X.shape[1]))
        history['epoch_num'].append(0.)
        history['func'].append(self.get_objective(X, y))
        history['time'].append(0.)
        history['weights_diff'].append(0.)
        history['accuracy'].append(np.mean(self.predict(X_test) == y_test))
        last_w = self.w.copy()
        start = time()
        for i in range(self.max_iter):
            # before_choice = time()
            offset = i % (X.shape[0] // self.batch_size)
            if(offset == 0):
                index = np.random.permutation(X.shape[0])
            subset = index[offset:offset+self.batch_size]
            # before_change = time()
            self.w -= (self.step_alpha / ((i + 1) ** self.step_beta)) * self.get_gradient(X[subset], y[subset])
            # before_epoch_calc = time()
            epoch = ((i + 1) * self.batch_size) / X.shape[0]
            # before_epoch_comp = time()
            if(epoch - history['epoch_num'][-1] >= log_freq):
                end = time()
                # print('choose:', choose)
                # print('change:', change)
                # print('ecalc:', ecalc)
                # print('ecomp:', ecomp)
                history['epoch_num'].append(epoch)
                history['func'].append(self.get_objective(X, y))
                history['time'].append(end - start)
                history['weights_diff'].append(np.linalg.norm(self.w - last_w) ** 2)
                history['accuracy'].append(np.mean(self.predict(X_test) == y_test))
                last_w = self.w.copy()
                if((abs(history['func'][-1] - history['func'][-2]) < self.tolerance)
                   or (abs(history['weights_diff'][-1]) ** 0.5 < self.tolerance)):
                    break
                start = time()
            # the_end = time()
            # choose += before_change - before_choice
            # change += before_epoch_calc - before_change
            # ecalc += before_epoch_comp - before_epoch_calc
            # ecomp += the_end - before_epoch_comp
        if(trace):
            return history

    def predict(self, X):
        """
        Получение меток ответов на выборке X

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        return: одномерный numpy array с предсказаниями
        """
        probs = self.predict_proba(X)
        res = np.argmax(probs, axis=1)
        if(self.loss_function == 'binary_logistic'):
            return 2 * res - 1
        else:
            return res

    def predict_proba(self, X):
        """
        Получение вероятностей принадлежности X к классу k

        X - scipy.sparse.csr_matrix или двумерный numpy.array

        return: двумерной numpy array, [i, k] значение соответветствует вероятности
        принадлежности i-го объекта к классу k
        """
        if(self.loss_function == 'binary_logistic'):
            probs = expit(X.dot(self.w.T).ravel())
            return np.vstack([1 - probs, probs]).T
        else:
            probs = X.dot(self.w.T)
            probs = np.exp(probs - probs.max(axis=1)[:, np.newaxis])
            probs /= probs.sum(axis=1)[:, np.newaxis]
            return probs

    def get_objective(self, X, y):
        """
        Получение значения целевой функции на выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array
        y - одномерный numpy array

        return: float
        """
        return self.clf.func(X, y, self.w)

    def get_gradient(self, X, y):
        """
        Получение значения градиента функции на выборке X с ответами y

        X - scipy.sparse.csr_matrix или двумерный numpy.array
        y - одномерный numpy array

        return: numpy array, размерность зависит от задачи
        """
        return self.clf.grad(X, y, self.w)

    def get_weights(self):
        """
        Получение значения весов функционала
        """
        return self.w


if(__name__ == '__main__'):
    np.random.seed(30)
    clf = SGDClassifier(loss_function='binary_logistic', batch_size=5, step_alpha=1, step_beta=0,
                        tolerance=1e-4, max_iter=10000000, l2_coef=0.1, random_seed=153)

    l, d = 100000, 10
    X = np.random.random((l, d))
    y = np.random.randint(0, 2, l) * 2 - 1
    w = np.random.random(d)
    history = clf.fit(X, y, w_0=np.zeros(d), trace=True, log_freq=1)
    print('\n'.join([str(x) for x in history['func']]))
