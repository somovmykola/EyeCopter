close all;

filepath = './';
filename = 'data.txt';

fID = fopen([filepath filename], 'r');

[data, len] = fscanf(fID, '%f %f %f\r\n');

fclose(fID);


T = data(1:3:len);
goalY = data(2:3:len);
realY = data(3:3:len);


figure;
hold on;
plot(T, goalY, T, realY);

xlabel('time (s)');
ylabel('height (m)');

hold off;