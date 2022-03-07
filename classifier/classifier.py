import torch.nn as nn
import torch.nn.functional as F
from dgl.nn import GraphConv
import dgl
from dataclasses import dataclass
import torch


@dataclass
class ClassifierConfig:
    num_embeddings: int = 512  # vocab size
    embedding_dim: int = 64
    # in_dim: int = 1
    hidden_dim: int = 128
    n_classes: int = 106


class Classifier(nn.Module):
    def __init__(self, config: ClassifierConfig):
        super().__init__()

        self.embedding = nn.Embedding(num_embeddings=config.num_embeddings, embedding_dim=config.embedding_dim)
        # self.conv1 = GraphConv(config.in_dim, config.hidden_dim, allow_zero_in_degree=True)
        self.conv1 = GraphConv(config.embedding_dim, config.hidden_dim, allow_zero_in_degree=True)
        self.conv2 = GraphConv(config.hidden_dim, config.hidden_dim, allow_zero_in_degree=True)
        self.classify = nn.Linear(config.hidden_dim, config.n_classes)

    def forward(self, g):
        # h = g.in_degrees().view(-1, 1).float()
        h = self.embedding(g.ndata['text'])

        h = F.relu(self.conv1(g, h))
        h = F.relu(self.conv2(g, h))

        g.ndata['h'] = h
        hg = dgl.max_nodes(g, 'h')

        return self.classify(hg)


'''
class Classifier(nn.Module):
    def __init__(self, config: ClassifierConfig):
        super().__init__()

        self.embedding = nn.Embedding(num_embeddings=config.num_embeddings, embedding_dim=config.embedding_dim)
        # self.conv1 = GraphConv(config.in_dim, config.hidden_dim, allow_zero_in_degree=True)
        self.conv1 = GraphConv(config.embedding_dim, config.hidden_dim//4, allow_zero_in_degree=True)
        self.conv2 = GraphConv(config.hidden_dim//4, config.hidden_dim//2, allow_zero_in_degree=True)
        self.conv3 = GraphConv(config.hidden_dim//2, config.hidden_dim//2, allow_zero_in_degree=True)
        self.linear = nn.Linear(config.hidden_dim, config.hidden_dim)
        self.classify = nn.Linear(config.hidden_dim, config.n_classes)

    def forward(self, g):
        h = self.embedding(g.ndata['text'])

        h = F.relu(self.conv1(g, h))
        h = F.relu(self.conv2(g, h))
        h = F.relu(self.conv3(g, h))

        g.ndata['h'] = h
        hg = torch.cat((dgl.max_nodes(g, 'h'), dgl.mean_nodes(g, 'h')), dim=-1)
        hg = F.relu(self.linear(hg))

        return self.classify(hg)
'''