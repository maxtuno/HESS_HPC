import sys
import numpy
import matplotlib.pyplot as plt

data = numpy.loadtxt(sys.argv[1])

plt.figure(figsize=(10, 10))
plt.title('HESS polynomial black-box optimization algorithm https://twitter.com/maxtuno')
x, y = zip(*data)
plt.plot(x, y, 'r.')
plt.plot(x, y, 'k-')
plt.savefig(sys.argv[1] + '_tour.png')

plt.figure(figsize=(10, 10))
plt.title('HESS polynomial black-box optimization algorithm https://twitter.com/maxtuno')
x, y = zip(*data)
plt.plot(x, y, 'r.')
plt.savefig(sys.argv[1] + '.png')
