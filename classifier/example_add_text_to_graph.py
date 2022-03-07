import programl as pg
import torch
from utils import Vocab

CODE = "int f(int x, int y) {return x+y;}"

if __name__ == '__main__':
    graph = pg.from_cpp(CODE, timeout=10)
    dgl_graph = pg.to_dgl(graph)
    vocab = Vocab()
    dgl_graph.ndata['text'] = torch.tensor(list(map(lambda x: vocab.add(x.text), graph.node)))
    print(list(map(lambda x: x.text, graph.node)))
    print(dgl_graph)
    print(dgl_graph.ndata['text'])
