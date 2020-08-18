%% Tactile Time Series Encoding with Fourier Basis Expansion and Higher Orthogonal Iteration of Tensors
%% Prepare Dataset
% Load the files *fc1.mat* and *fc2.mat* that contain the data structs.
close all
clear all

fc1 = load('fc1.mat'); % For classification and analysis
% fc2 = load('fc2.mat'); % For verification on new classes

n_cores = [3, 1, 0];
num_neighbors = 10;
voption = 0;

[~, Z, Zt, train_labels, test_labels] = tacProcess(fc1, n_cores, voption, 0.9);

class_names = unique(string(fc1.class_names).strip('both'));
pressures = unique(train_labels(:, 2));
class_ids = unique(train_labels(:, 1));

Vt = zeros(3, length(class_ids));
V0 =zeros(size(Vt));
t = zeros(1, length(Z));
r = zeros(size(t));

%% Compute line representation
for uc=class_ids'
    inds = find(train_labels(:, 1) == uc);
    [COEFF] = pcacov(Z(inds, :));
    z_ = Z(inds, :)';
    vt_= COEFF(:, 1);
    v0_= mean(z_, 2);
    v0_ = v0_ + dot(-v0_, vt_) * vt_;
    t_ = dot(repmat(v0_, 1, length(z_)) + z_, repmat(vt_, 1, length(z_)));
    
    Vt(:, uc) = vt_;
    V0(:, uc) = v0_;
    t(inds) = t_;
    r(inds) = sqrt(sum((z_ - v0_).^2, 1) - t_.^2);
end

figure(1)
hold on
gscatter(train_labels(:, 2), atan(r./t)*180/pi, class_names(train_labels(:, 1)));
xlabel("Pressure")
ylabel("Deviation Angle (deg)")
hold off

np = 0.05;
figure(2)
hold on
gscatter(train_labels(:, 2), t'./(double(train_labels(:, 2)).^np), class_names(train_labels(:, 1)));
hold on;
xlabel("Pressure $p$", "Interpreter", "latex")
ylabel("$t/p^{"+string(np)+"}$", "Interpreter", "latex")
hold off

% for p=pressures'
%     inds = find(train_labels(:, 2) == p);
%     
%     figure(find(pressures == p)); hold on
%     gscatter(t(inds), r(inds), class_names(train_labels(inds, 1)));
%     title("P"+string(p));
%     xlabel("parameter (t)")
%     ylabel("distance to the line (r)")
% end

%% Test
% yt = zeros(length(Zt), 1);
% 
% for i = 1:length(Zt)
%     x = Zt(i, :)' - V0;
%     tx = dot(x, Vt, 1);
%     rx = sqrt(sum(x.^2, 1) - tx.^2);
%     label = test_labels(i, :);
%     
%     p = label(2);
%     inds = find(train_labels(:, 2) == p);
%     
%     figure(find(pressures == p)); hold on
%     gscatter(t(inds), r(inds), class_names(train_labels(inds, 1)), 'ymcrgbk');
%     gscatter(tx, rx, class_ids, 'ymcrgbk', '*');
%     title("P"+string(p));
%     xlabel("parameter (t)")
%     ylabel("distance to the line (r)")
% end