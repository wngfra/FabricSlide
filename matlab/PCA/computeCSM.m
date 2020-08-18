function [R_bar] = computeCSM(ellips)
%COMPUTECSM compute the cluster dispersion measure function for given
%clustering
%   

N = length(ellips);
count = 0;
R = 0;

for i=1:N
    e1 = ellips(i);
    m1 = e1.M;
    a1 = e1.A;
    s1 = sqrt(1/prod(eig(a1)));
    
    for j=i+1:N
        e2 = ellips(j);
        m2 = e2.M;
        a2 = e2.A;
        s2 = sqrt(1/prod(eig(a2)));
        R = R + (s1 + s2) / norm(m1 - m2);
        count = count + 1;
    end
end

R_bar = R / count;

end

