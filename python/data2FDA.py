import numpy as np
import scipy.io

from torchvision import transforms
from TacDataset import Normalize, TacDataset, ToFDA


n_basis = 17
n_channel = 16
period = 1


def extractCoeffs(ds):
    fcoeffs = np.zeros((n_basis, n_channel, len(ds)))

    for i in range(len(ds)):
        sample, _ = ds[i]
        fcoeffs[:, :, i] = sample

    return fcoeffs


def fda2matlab(root_dir, dst, dst_dir, transform):
    ds = TacDataset(root_dir, transform=tf)
    fc = extractCoeffs(ds)
    scipy.io.savemat(dst, mdict={'fbasis_coeffs': fc, 'class_ids': ds.get_class_ids(
    ), 'class_names': ds.get_class_names(), 'params': ds.get_params()}, oned_as='column')

    try:
        shutil.move(dst, dst_dir)
    except shutil.Error as e:
        print(e)
        os.remove(os.path.join(dst_dir, dst))
        shutil.move(dst, dst_dir)


if __name__ == "__main__":
    import os
    import shutil

    tf = transforms.Compose(
        [Normalize(axis=1), ToFDA(basis='Fourier', n_basis=n_basis, period=period)])
    # tf = transforms.Compose([ToSequence(sequence_length, stride), ToFDA(basis='Fourier', n_basis=n_basis, period=period)])

    dst_dir = '../matlab/HOOI'
    fda2matlab('../data/fabric', 'fc1.mat', dst_dir, tf)
    fda2matlab('../data/board', 'fc2.mat', dst_dir, tf)
