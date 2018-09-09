clc, clear

s = serial('COM3','BaudRate',9600);    % deklaracja portu i predkosci
%fopen(s);                             % otwieranie portu (tylko raz!)
%fclose(s)                             % zamykanie portu
t = 0:0.05:2*pi;                       % czas (6.28 (2*pi))

krok = 2000;

traj1=round(krok*sin(t));       
traj2=round(krok*sin(t+2*pi/3));
traj3=round(krok*sin(t+4*pi/3));

kroki1=traj1(2:end)-traj1(1:end-1);
kroki2=traj2(2:end)-traj2(1:end-1);
kroki3=traj3(2:end)-traj3(1:end-1);

for i=1:length(kroki1)
    fprintf(s,'x %d y %d z %d\n',[kroki1(i),kroki2(i),kroki3(i)]);
    pause(0.05)
end