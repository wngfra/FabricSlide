function [T, Z, Zt, train_labels, test_labels] = tacProcess(fc, n_cores, voption, perc)
%TACPROCESS process the tactile samples via HOOI to extract latent features and encode the 3D data
%
%   usage:
%   TACPROCESS(fc, n_cores, batch_last) process dataset fc with factor
%   matrix truncation configuration of n_cores.
%   Return HOOI result ttensor T, encoded training matrix Z, encoded
%   testing matrix Zt, grouped training labels train_labels and grouped
%   testing labels test_labels.
%
%   Alexander Wang
%   23/03/2019
%
%   Arguments
%   fc: dataset structure with 3D raw data
%   n_cores: 3D vector as factor-matrix truncation config

if nargin < 3
    voption = 0;
end

fd_coeffs = fc.fbasis_coeffs;
class_ids = fc.class_ids;

[r1, r2, r3] = size(fd_coeffs);
fd_cov = zeros(r2, r2, r3);

for i3=1:r3
    fd_cov(:, :, i3) = cov(fd_coeffs(:, :, i3));
    % fd_cov(:, :, i3) = fd_coeffs(:, :, i3) - mean(fd_coeffs(:, :, i3), 'all');
end

% Split dataset
random_idx = randperm(length(class_ids));
train_size = floor(length(class_ids)*perc);
train_idx = random_idx(1:train_size);
test_idx = random_idx(train_size+1:end);

pressure_labels = fc.params(:, 1);
speed_labels = fc.params(:, 2);
trainset = fd_cov(:, :, train_idx);
testset = fd_cov(:, :, test_idx);
train_labels = [class_ids(train_idx), pressure_labels(train_idx), speed_labels(train_idx)];
test_labels = [class_ids(test_idx), pressure_labels(test_idx), speed_labels(test_idx)];

switch voption
    case 1
        trainset = trainset(:, :, train_labels(:, 3) > 0);
        testset = testset(:, :, test_labels(:, 3) > 0);
        train_labels = train_labels(train_labels(:, 3) > 0, :);
        test_labels = test_labels(test_labels(:, 3) > 0, :);
        % disp('Use samples with positive velocities only')
    case -1
        trainset = trainset(:, :, train_labels(:, 3) < 0);
        testset = testset(:, :, test_labels(:, 3) < 0);
        train_labels = train_labels(train_labels(:, 3) < 0, :);
        test_labels = test_labels(test_labels(:, 3) < 0, :);
        % disp('Use samples with negative velocities only')
    otherwise
        % disp('Use all samples');
end

% Compute HOOI
fs3D = tensor(trainset);
T = tucker_als(fs3D, n_cores, struct('init', 'eigs', 'printitn', 0));
% Encode training samples with computed Tucker decomposition
Z = encodeTensor(fs3D, T, n_cores);
% Encode test sampless
fs3Dt = tensor(testset);
Zt = encodeTensor(fs3Dt, T, n_cores);

if n_cores(1) > 1
    Z = Z';
    Zt = Zt';
end

end