import numpy as np
from sklearn.neighbors import NearestNeighbors


class KNNClassifier:

    def __init__(self, k=1, strategy='my_own', metric='euclidean', weights=False, test_block_size=1000):
        """Конструктор класса.
• k — число ближайших соседей в алгоритме ближайших соседей
• strategy — алгоритм поиска ближайших соседей. Может принимать следующие значения:
– ’my_own’ — собственная реализация
– ’brute’ — использование sklearn.neighbors.NearestNeighbors(algorithm=’brute’)
– ’kd_tree’ — использование sklearn.neighbors.NearestNeighbors(algorithm=’kd_tree’)
– ’ball_tree’ — использование sklearn.neighbors.NearestNeighbors(algorithm=’ball_tree’)
• metric — название метрики, по которой считается расстояние между объектами. Может прини-
мать следующие значения:
– ’euclidean’ — евклидова метрика
– ’cosine’ — косинусная метрика
• weights — переменная типа bool. Значение True означает, что нужно использовать взвешенный
метод k ближайших соседей. Во взвешенном методе ближайших соседей голос одного объекта
равен 1/(distance + ε), где ε = 10^−5.
• test_block_size — размер блока данных для тестовой выборки"""
        self.n_neighbors = k
        self.strategy = strategy
        self.metric = metric
        self.weights = weights
        self.test_block_size = test_block_size

    def fit(self, X, y):
        """Описание параметров:
• X — обучающая выборка объектов
• y — ответы объектов на обучающей выборке
Метод производит обучение алгоритма с учётом стратегии указанной в параметре strategy."""
        if(self.strategy == 'my_own'):
            self.train = X.copy()
            self.train_2 = (self.train ** 2).sum(axis=1)
        else:
            self.train = NearestNeighbors(n_neighbors=self.n_neighbors, algorithm=self.strategy, metric=self.metric)
            self.train.fit(X)

        self.y = y.copy()

    def find_kneighbors(self, X, return_distance=False):
        """Описание параметров:
• X — выборка объектов
• return_distance — переменная типа bool
Метод возвращает tuple из двух numpy array размера (X.shape[0], k). [i, j] элемент первого мас-
сива должен быть равен расстоянию от i-го объекта, до его j-го ближайшего соседа. [i, j] элемент
второго массива должен быть равен индексу j-ого ближайшего соседа из обучающей выборки для
объекта с индексом i.
Если return_distance=False, возвращается только второй из указанных массивов. Метод должен
использовать стратегию поиска указанную в параметре класса strategy."""
        if(X.shape[0] > self.test_block_size):
            neighs = np.array([], dtype=int).reshape((0, self.n_neighbors))
            if(return_distance):
                neighs = (neighs, neighs)
                for i in range(X.shape[0]//self.test_block_size):
                    block = self.find_kneighbors(X[i*self.test_block_size:
                                                 (i+1)*self.test_block_size],
                                                 return_distance=True)
                    neighs = (np.concatenate((neighs[0], block[0])), np.concatenate((neighs[1], block[1])))

                if(X.shape[0] % self.test_block_size != 0):
                    block = self.find_kneighbors(X[(X.shape[0]//self.test_block_size)
                                                 * self.test_block_size:],
                                                 return_distance=True)
                    neighs = (np.concatenate((neighs[0], block[0])), np.concatenate((neighs[1], block[1])))
            else:
                for i in range(X.shape[0]//self.test_block_size):
                    neighs = np.concatenate((neighs, self.find_kneighbors(X[i*self.test_block_size:
                                                                          (i+1)*self.test_block_size],
                                                                          return_distance=False)))
                if(X.shape[0] % self.test_block_size != 0):
                    neighs = np.concatenate((neighs, self.find_kneighbors(X[(X.shape[0]//self.test_block_size)
                                                                          * self.test_block_size:],
                                                                          return_distance=False)))
            return neighs

        if(self.strategy == 'my_own'):
            if(self.metric == 'euclidean'):
                dists_2 = (X ** 2).sum(axis=1)[:, np.newaxis] + self.train_2[np.newaxis, :] - 2*np.dot(X, self.train.T)
                ind = np.argpartition(dists_2, range(self.n_neighbors))[:, :self.n_neighbors]
                if(return_distance):
                    return (dists_2[np.repeat(np.arange(X.shape[0]), self.n_neighbors),
                            ind.flatten()].reshape(ind.shape) ** 0.5, ind)
                else:
                    return ind
            else:
                norm = np.outer((X ** 2).sum(axis=1), self.train_2) ** 0.5
                norm = np.where(np.isclose(norm, 0), 1, norm)
                dists = 1 - np.dot(X, self.train.T)/norm
                ind = np.argpartition(dists, range(self.n_neighbors))[:, :self.n_neighbors]
                if(return_distance):
                    return (dists[np.repeat(np.arange(X.shape[0]), self.n_neighbors),
                            ind.flatten()].reshape(ind.shape), ind)
                else:
                    return ind
        else:
            return self.train.kneighbors(X, return_distance=return_distance)

    def predict(self, X):
        """Описание параметров:
• X — тестовая выборка объектов
Метод должен вернуть одномерный numpy array размера X.shape[0], состоящий из предсказаний
алгоритма (меток классов) для объектов тестовой выборки."""
        neighs = self.find_kneighbors(X, return_distance=self.weights)

        if(self.weights):
            y_neighs = self.y[neighs[1].flatten()].reshape(neighs[1].shape)
            dist_neighs = 1 / (neighs[0] + 10e-5)
            result = np.zeros(X.shape[0], dtype=int)
            scores = np.zeros((X.shape[0],))
            for c in np.unique(y_neighs):
                c_scores = np.where(y_neighs == c, dist_neighs, 0.).sum(axis=1)
                result = np.where(c_scores > scores, c, result)
                scores = np.maximum(scores, c_scores)
            return result
        else:
            y_neighs = self.y[neighs.flatten()].reshape(neighs.shape)
            result = np.zeros(X.shape[0], dtype=int)
            scores = np.zeros((X.shape[0],))
            for c in np.unique(y_neighs):
                c_scores = np.where(y_neighs == c, 1, 0).sum(axis=1)
                result = np.where(c_scores > scores, c, result)
                scores = np.maximum(scores, c_scores)
            return result
