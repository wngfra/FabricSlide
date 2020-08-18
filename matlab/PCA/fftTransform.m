function [M] = fftTransform(ds)
%FFTTRANSFORM Convert time series to frequency spectrum
% Extract features from the frequency power spectrum
ch = 4:19;
sliding_length = 500;
ref_vel = 50;

for i = length(ds):-1:1
    velocity = abs(ds(i).velocity);

    data = resample(ds(i).data(:, ch), velocity, ref_vel);
    data = data(17:17+floor(sliding_length/ref_vel*36), :);
    demeaned = data - mean(data);
    
    Ys = fft(demeaned / length(demeaned));
    Ys = Ys - mean(Ys);
    Fs = [real(Ys);imag(Ys)];
    
    M(i, :) = Fs(:)'; % Store all data
end

end
