import programl as pg
import traceback
import pandas as pd
import numpy as np
import os
import glob
from multiprocessing import Pool, Lock
import torch
import utils

TRAIN = 0
TEST = 1
VALID = 2

USE_THREADS = False
MAX_DATA_PREP = 1000000000

# internet told me so
torch.multiprocessing.set_sharing_strategy('file_system')

lock_2 = Lock()
started = 0
finished = 0

lock_vocab = Lock()
vocab = None

NUM_PER_CLASS_REDUCED = 20 # This should be the same as the number of FFTs available.
def reduce_source(source):
    by_class = {}
    for ind, item in source.iterrows():
        label = item.label
        if label in by_class:
            if len(by_class[label]) < NUM_PER_CLASS_REDUCED:
                by_class[label].append(item)
        else:
            by_class[label] = [item]

    # Now, rebuild the source:
    new_data = []
    for item in by_class.keys():
        new_data += by_class[item]

    return pd.DataFrame(new_data)

def parse_code(code, mode):
    global vocab
    lock_2.acquire()
    global started
    started += 1
    lock_2.release()
    # get more code working
    modified_code = "#include<assert.h>\n#include<stdio.h>\n#include<iostream>\n#include<math.h>\n#include<string.h>\nusing namespace std;\n" + code
    # print ("Code is ", code)
    # print ("Label is ", label)
    # follow the programl tutorial
    graph = pg.from_cpp(modified_code, timeout=10)
    dgl_graph = pg.to_dgl(graph)
    global lock_vocab
    print ("Trying to acquire lock")
    lock_vocab.acquire()
    print ("Got the lock")
    if vocab is not None:
        if mode == TRAIN:
            print("Training mode data")
            dgl_graph.ndata['text'] = torch.tensor(list(map(lambda x: vocab.add(x.text), graph.node)))
        else:
            print ("Test mode data")
            # mode is valid/test
            dgl_graph.ndata['text'] = torch.tensor(list(map(lambda x: vocab.token2idx(x.text), graph.node)))

    print ("Releasing")
    lock_vocab.release()
    lock_2.acquire()

    global finished
    finished += 1
    lock_2.release()
    # print("Nodes is " + str(networkx.number_of_nodes()))

    return dgl_graph


lock = Lock()
deleted = 0
succeeded = 0

class RowPair:
    def __init__(self, rows, vocab):
        self.rows = rows
        self.vocab = vocab

def parse_file_train(inps):
    return parse_file(inps.rows, inps.vocab, TRAIN)

def parse_file_test(inps):
    return parse_file(inps.rows, inps.vocab, TEST)

def parse_file(rows, vocab, mode):
    assert vocab is not None
    global succeeded
    global deleted
    global loc
    num = 0

    # iterrows does a copy before generting elements
    new_rows = []
    for index, row in rows.iterrows():
        next_row = {}
        next_row['id'] = row['id']
        next_row['label'] = row['label']
        num += 1
        # if num > 10:
        #     break
        try:
            dgl_graph = parse_code(row['code'], mode) # , row['label'])

            next_row['code'] = dgl_graph

            # lock.acquire()
            succeeded += 1
            # lock.release()
            print ("Succesfully handled class " + str(next_row['label']))
        except Exception as e:
            print(e)
            # That one failed, so just delete it?
            next_row['code'] = None
            # lock.acquire()
            deleted += 1
            # lock.release()
            print (traceback.format_exc())
            print ("Failed class " + str(next_row['label']))

        # lock.acquire()
        print ("Number deleted is " + str(deleted))
        print ("Number retained is " + str(succeeded))
        # lock.release()
        new_rows.append(next_row)
    return pd.DataFrame(new_rows)


# Created with reference to ASTNN
class Pipeline:
    def __init__(self, root, use_fft):
        self.source = None
        self.root = root
        self.use_fft = use_fft
        self.vocab = utils.Vocab()

    def parse_source(self):
        # Now, parse all the inputs:
        def parse_values(vs, mode):
            global vocab
            vocab = self.vocab
            ncores = 15
            if USE_THREADS:
                with Pool(ncores) as p:
                    split = np.array_split(vs, ncores)
                    # some stupid hack because I don't remmeber how to pass a tuple
                    splits = []
                    for s in split:
                        pairs = RowPair(s, self.vocab)
                        splits.append(pairs)
                    if mode == TEST or mode == VALID:
                        result = pd.concat(p.map(parse_file_test, splits))
                    else:
                        result = pd.concat(p.map(parse_file_train, splits))
            else:
                result = parse_file(vs, self.vocab, mode)
            return result

            print("Finished processing!")

        self.train = parse_values(self.train, TRAIN)

        classes_gened = {}
        for _, source in self.train.iterrows():
            if source['label'] in classes_gened:
                classes_gened[source['label']] += 1
            else:
                classes_gened[source['label']] = 1
        print ("Training Classes generated were: " + str(classes_gened))

    # split data for training, developing and testing
    def split_data(self, input_file):
        path = self.root + input_file

        source = pd.read_pickle(path)
        source.columns = ['id', 'code', 'label']
        # Try to keep the numbers a bit balanced, we dn't have that many FFT examples.
        # Randomly order so we get roughly enough of these.
        source = source.sample(frac=1, random_state=1)
        source = source.head(n=MAX_DATA_PREP)
        if not self.use_fft:
            # running into memory issues with my crap scripts.
            source = source.head(n=15000)
        if self.use_fft:
            source = reduce_source(source)

            # Now add the FFT files
            fft_files = glob.glob("fft_data/*.c")
            max_index = source.loc[source['id'].idxmax()].id
            fft_label = source.loc[source['label'].idxmax()].label + 1
            print("FFT Label is " + str(fft_label))
            passed_ffts = 0

            for file in fft_files:
                print ("Looking at file ", file)
                max_index += 1
                with open(file) as f:
                    code = f.read()
                    try:
                        parsed = parse_code(code, TEST)
                        passed_ffts += 1
                    except:
                        pass

                source.loc[max_index] = [
                        max_index,
                        code,
                        fft_label
                ]

            print("Total sources considered is " + str(len(source)))
            print("Passed FFTs is " + str(passed_ffts))

        data = source
        data_num = len(data)
        train_split = int(data_num / 2)
        data_by_class = {}
        for _, item in data.iterrows():
            if item['label'] in data_by_class:
                data_by_class[item['label']].append(item)
            else:
                data_by_class[item['label']] = [item]

        train = []
        test = []
        valid = []
        # 1 : 10
        for item in data_by_class:
            l = data_by_class[item]
            split_point = len(l) // 10
            train += l[:]
            # Less in test
        self.train = pd.DataFrame(train)

    def write_data(self):
        def check_or_create(path):
            if not os.path.exists(path):
                os.mkdir(path)
        train_path = self.root+'train/'
        check_or_create(train_path)
        self.train_file_path = train_path+'train_.pkl'
        self.train.to_pickle(self.train_file_path)

        self.vocab.to_json(self.root + 'vocab.json')


if __name__ == "__main__":
    # pretrain_pipeline = Pipeline('pretrain_data/', False) # This is the non-fft data only.
    # print ("Parsing the OJClone code")
    # pretrain_pipeline.split_data(input_file='programs.pkl')
    # pretrain_pipeline.parse_source()
    # pretrain_pipeline.write_data()

    ppline = Pipeline('data/', True) # put the fft data in his one -- has to be a lot smaller to be balanced.
    print("Parsing source code")
    ppline.split_data(input_file='programs.pkl')
    ppline.parse_source()
    ppline.write_data()
