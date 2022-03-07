import numpy
import json
import sys

# 6
# RealIn = [78.1233661709, 76.2242588478, 77.7672396436, 78.3022165309, 34.0213473576, 45.8260651612, 65.5697454615, 42.672176301]
# ImagIn = [61.1449083736, 48.8763447243, 6.81035935781, 67.1144833462, 26.8160140873, 41.0745737701, 29.7606982563, 11.1374448067]
# 1
# RealIn =  [28.7158795237, 41.653811765, 77.2884866631, 38.4782461199]
# ImagIn = [34.6047109283, 2.39446181735, 82.8885180846, 16.4287584736]
# 10
with open(sys.argv[1]) as f:
    json = json.load(f)

# RealIn = json['InRealData']
# ImagIn = json['InImagData']

a = []
for i in range(len(json['a']) // 2):
    a.append(json['a'][2*i] + json['a'][2*i + 1] * 1j)

print ("Forward fft")
print (numpy.fft.fft(a,))
print ("IFFT")
print (numpy.fft.ifft(a,))
