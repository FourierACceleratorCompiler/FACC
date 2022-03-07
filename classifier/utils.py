import torch
import random
import numpy as np
import json
import dataclasses
import logging
from collections import OrderedDict
import json

vocab_id = 0


def init_logging(name: str):
    logging.basicConfig(filename=name, level=logging.INFO)
    logging.getLogger('').addHandler(logging.StreamHandler())


def set_seed(seed: int):
    torch.manual_seed(seed)
    random.seed(seed)
    np.random.seed(seed)
    torch.use_deterministic_algorithms(True)


class EnhancedJSONEncoder(json.JSONEncoder):
    def default(self, o):
        if dataclasses.is_dataclass(o):
            return dataclasses.asdict(o)
        return super().default(o)


class Vocab:
    def __init__(self):
        # print ("Starting new Vocab")
        self._token2idx = OrderedDict()
        self._token2idx['<UNK>'] = 0
        self._idx2token = ['<UNK>']
        global vocab_id
        self.id = vocab_id
        vocab_id += 1

    @classmethod
    def from_json(cls, json_path):
        with open(json_path, 'r') as f:
            pretrained_dict = json.load(f)
        vocab = Vocab()
        vocab._token2idx = OrderedDict(pretrained_dict['token2idx'])
        vocab._idx2token = pretrained_dict['idx2token']
        return vocab

    def add(self, token):
        # print ("Adding to ID " + str(self.id))
        # print ("Number of entries is " + str(len(self._token2idx)))
        # print (self._idx2token)
        if token in self._token2idx:
            return self._token2idx[token]
        idx = len(self._token2idx)
        self._token2idx[token] = idx
        self._idx2token.append(token)
        return idx

    def to_json(self, path):
        # print ("Output from ID " + (str(self.id)))
        # print (self._idx2token)
        print ("Outputting, number of ids is " + str(len(self._token2idx)))
        pretrained_dict = {'token2idx': self._token2idx, 'idx2token': self._idx2token}
        with open(path, 'w') as f:
            json.dump(pretrained_dict, f)

    def token2idx(self, token):
        if token in self._token2idx:
            return self._token2idx[token]
        return self._token2idx['<UNK>']

    def idx2token(self, idx):
        return self._idx2token[idx]
