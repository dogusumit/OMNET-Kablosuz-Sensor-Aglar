function donus = dosya_oku(dosya)
dosya=fopen(dosya,'r');
donus=fscanf(dosya,'%f',inf);
fclose(dosya);
end

