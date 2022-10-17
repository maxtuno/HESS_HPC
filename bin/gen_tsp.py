import sys
import numpy
import matplotlib.pyplot as plt

n = int(sys.argv[1])

data = numpy.random.logistic(size=(n, 2)) + numpy.random.normal(size=(n, 2))

print(n)
for i, (x, y) in enumerate(data):
    print(i + 1, x, y)

plt.figure(figsize=(10, 10))

plt.title('HESS polynomial black-box optimization algorithm https://twitter.com/maxtuno')
x, y = zip(*data)
plt.plot(x, y, 'r.')
plt.savefig(sys.argv[1] + '.png')