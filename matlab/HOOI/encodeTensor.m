function [encoding] = encodeTensor(X, T, ncores)
%ENCODETENSOR Encode tensor with given HOSVD result.
%   ENCODING = ENCODETENSOR(X, T, NCORES) encodes the input tensor X with
%   the HOSVD result T, according to the dimensions NCORES by trimming the
%   factor matrix of T. ENCODING is computed via n-mode product of tensor X
%   and the trimmed factor matrices. Results is an N-D matrix.
%  
%   Alexander Wang
%   23/03/2019
%
%   Example:
%       X = tensor(rand(100,100,100));
%       T = hosvd(X, 1e-2, 'verbosity', 0);
%       Z = encodeTensor(X, T, [3, 1, 0]);
    
% Store the compressed tensor
encoding = X;
% Compress tensor
if ncores(1) > 0
    encoding = ttm(encoding, transpose(T.u{1}), 1);
end
if ncores(2) > 0
    encoding = ttm(encoding, transpose(T.u{2}), 2);
end
% if ncores(3) > 0
%     encoding = ttm(encoding, transpose(T.u{3}), 3);
% end

encoding = abs(squeeze(encoding).data);

end

