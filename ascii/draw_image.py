import sys
import os
from PIL import Image
import numpy as np

# define the set of characters to use
chars = ' .:-=+*#%@$'

# get an image path
path = 'cat.jpg'
if len(sys.argv) > 1:
	path = sys.argv[1]

# get terminal size
rows, columns = os.popen('stty size', 'r').read().split()
rows, columns = int(rows), int(columns)

# load the image convert to grayscale
img = np.array(Image.open(path))
img = img.mean(axis=2)
img /= img.max()

scale = (img.shape[0] / rows, img.shape[1] / columns)

# print the image as ASCII characters
result = ''
for x in range(rows):
	for y in range(columns):
		index = (int(x * scale[0]), int(y * scale[1]))
		value = np.mean(img[index[0], index[1]])
		result += chars[int(value * len(chars)) - 1]
	result += '\n'
print(result, end='')
