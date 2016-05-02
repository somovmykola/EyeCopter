clc;
clear;
close all;

fID = fopen('data.txt', 'r');

maxRatio = 4;

X = [];
Y = [];

for i = 1:20
   n = fscanf(fID, '\r\nDistance: %dcm\r\n\r\n');
    
   if (n == -1)
       break;
   end
   
   data = fscanf(fID, '%d', 30);   
   diff = abs(data - median(data));
   avDiff = mean(diff);
       
   j = 1;
   while (j < length(data))
       if (diff(j) / avDiff > maxRatio)
           data(j) = [];
           diff(j) = [];
       else
           j = j + 1;
       end
   end   
   
   X = [X; ones(length(data),1) * n];
   Y = [Y; data];
end

Y = Y.*0.001;

figure;
hold on;
xlabel('Distance from wall (cm)');
ylabel('Time of Flight (mS)');

% Data points
scatter(X, Y, '.r');

% Line of Best Fit
p = polyfit(X, Y, 1);
f = polyval(p, X);

% y=mx+b
% m = 0.057309
% b = -0.191335

plot(X, f, '-b');

Rsq = 1 - sum((Y - f).^2) / sum((Y - mean(Y)).^2);
legend('raw data', sprintf('R^{2} = %.5f', Rsq));
legend('Location', 'best');

hold off;



