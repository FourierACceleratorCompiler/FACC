import pickle
import os
from simple_parsing import ArgumentParser
import torch
from classifier import Classifier, ClassifierConfig
from scipy import stats
from dgl.dataloading import GraphDataLoader
import torch.optim as optim
import torch.nn as nn
import pandas as pd
from dgl.data import DGLDataset
import matplotlib.pyplot as plt
from dataclasses import dataclass
from utils import set_seed, EnhancedJSONEncoder, init_logging
import git
import time
import uuid
import ntpath
import json
import logging
from torch.utils.tensorboard import SummaryWriter
import random

SEED = 42

set_seed(SEED)

USE_CUDA = False
DEVICE = torch.device('cuda:0') if torch.cuda.is_available() and USE_CUDA else torch.device('cpu')

PRETRAIN_DATA_PATH = 'pretrain_data'
TRAIN_DATA_PATH = 'data'
TRAIN_SET = os.path.join('train', 'train_.pkl')

# used to restrict the number fo data points for debugging.
MAX_DATA = 10000000000


@dataclass
class TrainConfig:
    classifier_config: ClassifierConfig
    lr: float = 0.0001
    wd: float = 0.005
    bs: int = 8
    max_epoch: int = 100
    patience: int = 10

    max_data_per_class: int = 20


class CodeDataset(DGLDataset):
    def __init__(self, fname, raw_dir=None, force_reload=False, verbose=False):
        self.fname = fname
        super(CodeDataset, self).__init__(name="OJClone", url='', raw_dir=raw_dir, force_reload=force_reload,
                                          verbose=verbose)

    # Uses an 80/20 split --- offset should be 0, 1, 2, 3, 4 to
    # rotate the data and get even coverage.
    def split(self, config, offset=0):
        rand = random.Random()
        rand.seed(0)

        data = {}
        for i in range(len(self.graphs)):
            lab = self.label[i]
            if lab in data:
                data[lab].append(self.graphs[i])
            else:
                data[lab] = [self.graphs[i]]

        # Now, shuffle according to rand within
        # each list.
        for i in data.keys():
            rand.shuffle(data[i])
            data[i] = data[i][:max(config.max_data_per_class, 3)]

        # Now, do a 8:2 split:
        train_graphs = []
        train_labels = []
        test_graphs = []
        test_labels = []
        
        assert offset < 10
        assert offset >= 0
        for i in data.keys():
            this_data = data[i]
            rot_by = (2) * offset
            this_data = this_data[rot_by:] + this_data[:rot_by]
            split_loc = len(this_data) - 2
            added = this_data[:split_loc]
            train_graphs += added
            train_labels += [i] * (len(added))

            test_added = this_data[split_loc:]
            test_graphs += test_added
            test_labels += [i] * (len(test_added))

        train_dataset = CodeDataset(self.fname)
        test_dataset = CodeDataset(self.fname)

        train_dataset.graphs = train_graphs
        train_dataset.label = train_labels
        test_dataset.graphs = test_graphs
        test_dataset.label = test_labels

        return train_dataset, test_dataset

    def process(self):
        logging.info("Loading!")
        # load the data
        inputs = pd.read_pickle(self.fname)
        # I think it shuffles elsewhere too?
        #traininputs = traininputs.sample(frac=1)

        self.graphs = []
        self.label = []
        num = 0
        for _, input in inputs.iterrows():
            logging.info(f"Last data was {input['label']}")
            logging.info(input['code'])
            # Avoid loading too much data
            if num > MAX_DATA:
                break
            # Code can be none if ProGraML failed to parse the C file properly e.g. missing deps
            if input['code'] is not None:
                num += 1
                dgl_graph = input['code']
                # Follow the instrctons of the error message
                # would really like to do some sort of check here,
                # not clear why there are nodes without any attached
                # edges in the graph..
                # print (dgl_graph.edata_schemes)
                # WHY TF IS THIS BROKEN
                #dgl_graph.add_self_loop(dgl_graph, etype=('_V'))
                self.graphs.append(dgl_graph)
                # label should be between 0 and 105
                self.label.append(input['label'])
        # Print the labels to check for some semblance of balance.
        label_dict = {}
        for l in self.label:
            if l in label_dict:
                label_dict[l] += 1
            else:
                label_dict[l] = 1
        logging.info("Loaded! Got " + str(num))
        logging.info("Mode label is " + str(stats.mode(self.label)))
        logging.info("Labels are " + str(label_dict))

    def __getitem__(self, idx):
        graph, label = self.graphs[idx], self.label[idx]
        assert graph is not None
        assert label is not None
        return graph, label

    def __len__(self):
        return len(self.graphs)


def evaluate(dataloader, model, loss_func, batch_size):
    correct = 0
    correct_top = 0
    total = 0
    total_loss = 0.0
    model.eval()
    ffts_correct = 0
    ffts_top_correct = 0
    ffts_used = 0
    fft_class_values = []
    for iter, (bh, label) in enumerate(dataloader):
        bh = bh.to(DEVICE)
        label = label.to(DEVICE)
        prediction = model(bh)
        loss = loss_func(prediction, label)
        total_loss += loss.detach().cpu().item()
        total_predictions_fft = 0
        total_predictions_fft_top = 0
        result = nn.functional.softmax(prediction).cpu()
        for i, item in enumerate(result):
            r = torch.argmax(item)
            r_top = torch.argsort(item).flip(0)[0:3]
            if label[i] == r:
                correct += 1
            total += 1
            if r == 105:
                total_predictions_fft += 1
            if 105 in r_top:
                total_predictions_fft_top += 1
            if label[i] == 105:  # FFT Label
                if r == 105:
                    ffts_correct += 1
                if 105 in r_top:
                    ffts_top_correct += 1
                ffts_used += 1
                fft_class_value = item[105]
                fft_class_values.append(fft_class_value)
            if label[i] in r_top:
                correct_top += 1

    results = {'accuracy': correct / total, 'loss': total_loss/((iter + 1)*batch_size),
            'recall_ffts': ffts_correct/ffts_used, 'fft_weights': fft_class_values,
            'accuracy_top': correct_top/total, 'recall_ffts_top': ffts_top_correct/ffts_used,
            'precision_ffts': ffts_correct/total_predictions_fft if total_predictions_fft > 0 else -1,
            'precision_ffts_top': ffts_top_correct/total_predictions_fft_top if total_predictions_fft_top > 0 else -1
            }
    if results['precision_ffts'] != -1:
        results['f1_ffts'] = 2*results['precision_ffts']*results['recall_ffts']/(results['precision_ffts']+results['recall_ffts'])
    else:
        results['f1_ffts'] = -1
    if results['precision_ffts_top'] != -1:
        results['f1_ffts_top'] = 2 * results['precision_ffts_top'] * results['recall_ffts_top'] / (
                results['precision_ffts_top'] + results['recall_ffts_top'])
    else:results['f1_ffts_top'] = -1

    return results


def log_results(results, add_text):
    for key in results:
        logging.info(f"{add_text} {key} was: {results[key]}")


def run_training(model, args, output_dir, data_dir):
    config: TrainConfig = args.options

    writer = SummaryWriter(log_dir=os.path.join(output_dir, 'tb'))

    inputs = CodeDataset(os.path.join(data_dir, TRAIN_SET))

    traininputs, testinputs = inputs.split(config, offset=args.offset)
    validinputs = testinputs # do valid = test?

    logging.info(f'Train length: {len(traininputs)} Test length: {len(testinputs)}')
    # keep track of min and max no of nodes in graph.

    # TODO --- create this from the real data?
    # trainset = MiniGCDataset(len(train_inputs), min(num_nodes), max(num_nodes))
    # TODO --- refine this to test just the FFTs.
    # testset = MiniGCDataset(len(test_inputs), min(num_nodes), max(num_nodes))

    best_checkpoint_path = os.path.join(output_dir, 'checkpoints', args.model_name + "." + "BEST.mod")

    data_loader = GraphDataLoader(traininputs, batch_size=config.bs, shuffle=True)
    data_loader_validation = GraphDataLoader(validinputs, batch_size=config.bs, shuffle=False)
    data_loader_test = GraphDataLoader(testinputs, batch_size=config.bs, shuffle=False)

    loss_func = nn.CrossEntropyLoss()

    model = model.to(DEVICE)
    optimizer = optim.Adam(model.parameters(), lr=config.lr, weight_decay=config.wd)

    best_valid_accuracy = 0.0
    used_patience = 0

    n_updates = 0

    for epoch in range(1, config.max_epoch):
        epoch_loss = 0
        model.train()

        # as in pretrain.
        logging.info(f"Epoch {str(epoch)}")
        correct = 0
        total = 0
        for iter, (bh, label) in enumerate(data_loader):
            bh = bh.to(DEVICE)
            label = label.to(DEVICE)
            prediction = model(bh)
            loss = loss_func(prediction, label)
            pred = prediction.argmax(dim=1, keepdim=True)
            correct_update = pred.eq(label.view_as(pred)).sum().cpu().item()
            correct += correct_update
            writer.add_scalar('Loss_batch/train', loss, n_updates)
            writer.add_scalar('Accuracy_batch/train', correct/label.shape[0], n_updates)
            total += label.shape[0]
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()
            epoch_loss += loss.detach().cpu().item()
            n_updates += 1

        valid_metrics = evaluate(data_loader_validation, model, loss_func, batch_size=config.bs)
        logging.info(f"Train loss was {epoch_loss/total}")
        logging.info(f"Train accuracy was {correct/total}")
        writer.add_scalar('Accuracy/train', correct/total, epoch)
        writer.add_scalar('Loss/train', epoch_loss, epoch)
        #logging.info(f"Validation loss was: {valid_metrics['loss']}")
        #logging.info(f"Validation accuracy was: {valid_metrics['accuracy']}")
        #logging.info(f"FFT Weights were: {valid_metrics['fft_weights']}")
        writer.add_scalar('Loss/valid', valid_metrics['loss'], epoch)
        writer.add_scalar('Accuracy/valid', valid_metrics['accuracy'], epoch)
        #logging.info(f"Validation FFT accuracy was: {valid_metrics['accuracy_ffts']}")
        #logging.info(f"Validation TOP accuracy was: {valid_metrics['accuracy_top']}")
        #logging.info(f"Validation FFT TOP accuracy was: {valid_metrics['accuracy_ffts_top']}")
        log_results(valid_metrics, 'Validation')
        if valid_metrics['accuracy'] > best_valid_accuracy:
            best_valid_accuracy = valid_metrics['accuracy']
            logging.info(f"Current best valid accuracy is: {best_valid_accuracy}")
            pickle.dump(model, open(best_checkpoint_path, "wb"))
            used_patience = 0
        else:
            logging.info(f"Current best valid accuracy is: {best_valid_accuracy}")
            used_patience += 1
            if used_patience == config.patience:
                logging.info(f'Early stopping after a patience of {used_patience}')
                break

        pickle.dump(model, open(os.path.join(output_dir, 'checkpoints',
                                             args.model_name + "." + (str(epoch)) + ".mod"), "wb"))

    # Load best model as per valid accuracy
    model = pickle.load(open(best_checkpoint_path, 'rb'))
    test_metrics = evaluate(data_loader_test, model, loss_func, batch_size=config.bs)
    #logging.info(f"Test loss was: {test_metrics['loss']}")
    #logging.info(f"Test accuracy was: {test_metrics['accuracy']}")
    #logging.info(f"Validation FFT accuracy was: {test_metrics['accuracy_ffts']}")
    #logging.info(f"FFT Prediction Weights were: {test_metrics['fft_weights']}")
    #logging.info(f"Test TOP accuracy was: {test_metrics['accuracy_top']}")
    #logging.info(f"Test TOP FFT accuracy was: {test_metrics['accuracy_ffts_top']}")
    log_results(test_metrics, 'Test')
    pickle.dump(model, open(args.model_name, 'wb'))


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument('model_name')
    parser.add_argument('offset', type=int)  # 0-9
    parser.add_arguments(TrainConfig, dest="options")
    parser.add_argument('--swap', dest='swap', help='swap train and test', action='store_true', default=False)
    parser.add_argument('--pretrain', dest='pretrain', help='pretrain the classifier', action='store_true',
                        default=False)
    parser.add_argument('--resume', dest='resume_file', help='resume training from previous file', action='store',
                        default=None)
    # Resume should only be used for reusing pretrained file, NOT to continue training

    args = parser.parse_args()

    config: TrainConfig = args.options

    timestamp = time.strftime("%Y-%m-%d-%H%M")
    output_path = 'output'
    pretrain = 'pretrain' if args.pretrain else 'train'
    model_name = ntpath.basename(args.model_name)
    repo = git.Repo(search_parent_directories=True)
    sha = repo.head.object.hexsha
    extra_id = uuid.uuid4().hex
    output_dir = os.path.join(output_path, f'{model_name}-{pretrain}-{timestamp}-{sha[:4]}-{extra_id[:4]}')

    os.makedirs(output_dir)
    os.makedirs(os.path.join(output_dir, 'checkpoints'))

    with open(os.path.join(output_dir, 'args.json'), 'w') as f:
        json.dump(vars(args), f, indent=4, cls=EnhancedJSONEncoder)

    init_logging(os.path.join(output_dir, 'train.log'))

    if args.resume_file is None:
        # create a new model --- 256 was the default, ASTNN uses a pooling layer
        # which IIUC we don't need for a reason I don't remember
        # I tried 600 also for the hidden layer, which I think ASTNN used
        # but it preformed a bit worse.
        model = Classifier(config.classifier_config)
    else:
        model = pickle.load(open(args.resume_file, 'rb'))

    data_dir = PRETRAIN_DATA_PATH if args.pretrain else TRAIN_DATA_PATH

    run_training(model, args, output_dir, data_dir=data_dir)
