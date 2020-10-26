import numpy as np
import pandas as pd

from tensorly.decomposition import tucker

from torchvision import transforms
from TacDataset import Normalize, TacDataset, ToFDA


n_basis = 33
n_channel = 16
period = 1


def extract_coeffs(ds):
    fcoeffs = np.zeros((n_basis, n_channel, len(ds)))

    for i in range(len(ds)):
        sample, _ = ds[i]
        fcoeffs[:, :, i] = sample

    return fcoeffs


def tucker_factorize(root_dir, transform):
    ds = TacDataset(root_dir, transform=tf)
    coeff_tensor = extract_coeffs(ds)
    _, factors = tucker(coeff_tensor, ranks=(3, 1, coeff_tensor.shape[2]))

    return np.array(factors, dtype=object)


if __name__ == "__main__":
    tf = transforms.Compose(
        [Normalize(axis=1), ToFDA(basis='Fourier', n_basis=n_basis, period=period)])
    # tf = transforms.Compose([ToSequence(sequence_length, stride), ToFDA(basis='Fourier', n_basis=n_basis, period=period)])

    factors = tucker_factorize('../data/fabric', tf)
    np.save('factors.npy', factors)