function ellip = conf_ellipsoid(X, N)
    M = mean(X)';
    X_bar = X - M';
    Cov = X_bar'*X_bar;

    [V,D] = eig(Cov);

    % For N standard deviations spread of data, the radii of the eliipsoid will
    % be given by N*SQRT(eigenvalues).

    radii = N*sqrt(diag(D));

    % generate data for "unrotated" ellipsoid
    [xc,yc,zc] = ellipsoid(0,0,0,radii(1),radii(2),radii(3));

    % rotate data with orientation matrix U and center M
    a = kron(V(:,1),xc); 
    b = kron(V(:,2),yc); 
    c = kron(V(:,3),zc);
 
    data = a+b+c; 
    n = length(data);
    nDn = n / 3; 
 
    % Quadric form of ellipsoids
    ellip.x = data(1:nDn,:)+M(1); 
    ellip.y = data(nDn+1:2*nDn,:)+M(2); 
    ellip.z = data(2*nDn+1:end,:)+M(3);
    ellip.M = M;
    ellip.A = V*diag(1./(radii.^2))/V;
    ellip.id = 0;