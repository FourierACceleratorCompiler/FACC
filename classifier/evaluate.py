import argparse
import pickle
import classifier
import prepare
import torch
import torch.nn as nn

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('model')
    parser.add_argument('c_file')

    args = parser.parse_args()

    model = pickle.load(open(args.model, 'rb'))

    # Create a graph for the C file:
    with open(args.c_file, 'r') as f:
        graph = prepare.parse_code(f.read())

    # Now, classify that graph:
    prediction = model(graph)
    result = torch.argmax(nn.functional.softmax(prediction))

    print("Got predicton " + str(result))
