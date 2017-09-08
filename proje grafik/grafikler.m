function sensor= grafikler(sensor,enerji)
a=dosya_oku(sensor);
b=dosya_oku(enerji);
c=b./a;

figure
subplot(3,1,1);
plot(a,'LineWidth',2);
xlabel('Dongu','FontSize',10,'FontWeight','normal');
ylabel('Aktif Sensor Sayisi','FontSize',10,'FontWeight','normal');
ylim([0 max(a)+1]);
xlim([0 size(a,1)+1]);
title(['Sistem ',num2str(size(a,1)),' Dongu Yasadi'],'fontweight','bold','fontsize',12);

subplot(3,1,2);
plot(b,'LineWidth',2);
xlabel('Dongu','FontSize',10,'FontWeight','normal');
ylabel('Harcanan Toplam Enerji ( joule )','FontSize',10,'FontWeight','normal');
ylim([0 max(b)+0.1]);
xlim([0 size(b,1)+1]);

subplot(3,1,3);
plot(c,'LineWidth',2);
xlabel('Dongu','FontSize',10,'FontWeight','normal');
ylabel('Harcanan Ortalama Enerji ( joule )','FontSize',10,'FontWeight','normal');
ylim([0 max(c)+0.01]);
xlim([0 size(c,1)+1]);
end