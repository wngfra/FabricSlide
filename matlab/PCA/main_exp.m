clear all
close all

%% Parameters
needplot = true;
n_components = 3;
forces = [120, 150, 180];
colors = distinguishable_colors(10);
ellip_size = 0.75;
rids = 1;

% Video setting
OptionZ.FrameRate=24;
OptionZ.Duration=6;
OptionZ.Periodic=true;

%% Load dataset
D1 = load('Fabrics5.mat');
D2 = load('Jeans2.mat');

dataset = [D1.all_data, D2.all_data];
labels = [D1.all_labels, D2.all_labels];

% Record all data
tab_exp = table();

%% Clustering and Classification
tic
%% Trainset
for train_n = 1:7
    for rid = 1:rids
        train_id = randsample(unique(labels), train_n);
        clear selected samples tags ellips
        for fid=length(forces):-1:1
            selected = [dataset(:).force]==forces(fid) & ismember([dataset(:).id], train_id) & [dataset(:).no]<=1;
            samples = dataset(selected);
            tags = labels(selected);
            
            fs = fftTransform(samples);
            [coeff, ~, latent] = pca(fs);
            pcs{fid} = coeff(:, 1:n_components); % Select principal components
            latent_score = sum(latent(1:n_components))/sum(latent);
            
            if needplot
                F(fid) = figure(fid); set(F(fid), "Name", "PCA Space");
                hold on
            end
            
            ellips = [];
            
            utags = unique(tags);
            for i=length(utags):-1:1
                eid = utags(i);
                esample = fs(eid == tags, :) * pcs{fid};
                ellip = conf_ellipsoid(esample, ellip_size);
                ellip.id = eid;
                ellips = [ellips, ellip];
                
                if needplot
                    labstr(i) = "fabric " + num2str(eid); % label string
                    % Plot the ellipsoid
                    sf(i) = surface(ellip.x, ellip.y, ellip.z, ...
                        'FaceColor', colors(eid, :), ...
                        'EdgeColor', 'none', ...
                        'FaceAlpha', 0.2, ...
                        'EdgeAlpha', 0.1, ...
                        'ButtonDownFcn', @(src, ~) msgbox(labstr(eid), 'Class label', 'replace'));
                    
                    scatter3(esample(:, 1), esample(:, 2), esample(:, 3), 20, colors(eid, :), "Marker", "o");
                end
            end
            if needplot
                legend(sf, labstr)
            end
            
            ellipsoid{fid} = ellips; % Store ellipsoids for each force
            R_bar1(fid) = computeCSM(ellips);
        end
        
        %% Testset
        
        for fid=length(forces):-1:1
            clear fs
            
            selected = [dataset(:).force]==forces(fid) & (~ismember([dataset(:).id], train_id) | [dataset(:).no]>1);
            samples = dataset(selected);
            tags = labels(selected);
            
            fs = fftTransform(samples);
            
            if needplot
                figure(fid); hold on
            end
            
            Es = ellipsoid{fid}; % Corresponding ellipsoid
            
            ellips = [];
            
            utags = unique(tags);
            for i=length(utags):-1:1
                eid = utags(i);
                if ~ismember(eid, train_id)
                    esample = fs(eid == tags, :) * pcs{fid};
                    ellip = conf_ellipsoid(esample, ellip_size);
                    ellip.id = eid;
                    ellips = [ellips, ellip];
                end
            end
            Es = [Es, ellips];
            
            R_bar2(fid) = computeCSM(Es);
            if needplot
                title("PCA Space for Force Value=" + num2str(forces(fid)) + ", $\bar{R} = $" + R_bar2(fid), "Interpreter", "latex")
            end
            
            %% Classify
            clear esample
            shot = 0;
            overshot = 0;
            for tag=unique(tags)
                esample = fs(tag == tags, :) * pcs{fid};
                if needplot
                    scatter3(esample(:, 1), esample(:, 2), esample(:, 3), 30, colors(tag, :), "Marker", "x");
                end
                
                for j=1:length(esample)
                    clear dist
                    x = esample(j, :);
                    
                    E = Es([Es.id] == tag);
                    dist = (x' - E.M)' * E.A * (x' - E.M);
                    shot = shot + (dist <= 1);
                    
                    os_count = 0;
                    for E=Es([Es.id] ~= tag)
                        dist = (x' - E.M)' * E.A * (x' - E.M);
                        os_count = os_count + (dist <= 1);
                    end
                    overshot = overshot + (os_count > 0);
                end
            end
            shot_rate(fid) = shot / length(tags);
            overshot_rate(fid) = overshot / length(tags);
            
            % CaptureFigVid([-20,10;-110,10;-190,80;-290,10;-380,10], "pca_f="+num2str(forces(fid))+"_train_n="+num2str(train_n), OptionZ)
        end
        
        % Record experimental data in a table
        id_list = zeros(1, 10);
        id_list(train_id) = 1;
        tab_tmp = table(train_n, id_list, R_bar1, R_bar2, shot_rate, overshot_rate);
        tab_exp = [tab_exp; tab_tmp];
        
%         hold off
%         close all
    end
end
toc

writetable(tab_exp, 'exp_results.csv', 'Delimiter', ',')

%% Plot results
stackedplot(tab_exp, ["R_bar1", "R_bar2", "shot_rate", "overshot_rate"])

%% Analysis
real_rate = table2array(tab_exp(:, 5)) - table2array(tab_exp(:, 6));
R_bar = table2array(tab_exp(:, 4));
[sorted_rate, I] = sort(real_rate, 2, "descend");
[~, J] = sort(R_bar, 2, "ascend");
count1 = sum(I(:, 1) == 1);
count2 = sum(I(:, 1) == 2);
count3 = sum(I(:, 1) == 3);
figure();hold on
title("Best true classification rate count for parametric settings")
xlabel("force value settings")
ylabel("count")
xbar = [120, 150, 180];
ybar = [count1, count2, count3];
bar(xbar, ybar)
labels = arrayfun(@(value) num2str(value), ybar, 'UniformOutput', false);
text(xbar,ybar,labels,...
    'HorizontalAlignment','center',...
    'VerticalAlignment','bottom')
hold off

%%
for k=0:4
    ids = (1:rids) + k * rids;
    avg(k+1, :) = sum(real_rate(ids, :), 1) / rids;
    avg_r(k+1, :) = sum(R_bar(ids, :), 1) / rids;
end
figure();hold on
title("Average correct rate of classification")
xlabel("Number of classes for training")
ylabel("Correct rate (%)")
bar(1:5, avg, 1)
legend("120", "150", "180", "Location", "northwest")

figure();hold on
title("Average Cluster Separation Measure (CSM)")
xlabel("Number of classes for training")
ylabel("CSM")
bar(1:5, avg_r, 1)
legend("120", "150", "180")
num_of_train_classes = [2;3;4;5;6];
stattable = table(num_of_train_classes, avg, avg_r);
writetable(stattable, "stat.csv", "Delimiter", ",");