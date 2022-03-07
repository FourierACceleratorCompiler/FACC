import json
import sys

with open(sys.argv[1]) as jsonfile:
    json = json.load(jsonfile)

for key in json:
    print ("Key", key)
    if hasattr(json[key], '__iter__'):
        for elt in json[key]:
            print (elt)
    else:
        print (json[key])
