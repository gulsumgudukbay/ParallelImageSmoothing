
close all;

fid = fopen('image.txt', 'r');
tline = fgetl(fid);
size = str2num(tline);

img = fscanf(fid, '%d', [size, size]);
fclose(fid);
poke = mat2gray(img);
figure; imshow( poke );

fid_out = fopen('output_image.txt', 'r');
img_out = fscanf(fid_out, '%d', [size, size]);
fclose(fid_out);
poke_out = mat2gray(img_out);
figure; imshow( poke_out );