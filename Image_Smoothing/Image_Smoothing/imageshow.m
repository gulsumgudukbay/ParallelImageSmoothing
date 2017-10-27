
close all;

fid = fopen('image.txt', 'r');
tline = fgetl(fid);
size = str2num(tline);

img = fscanf(fid, '%f', [size, size]);
fclose(fid);
img = transpose(img);
z = img(32,1)+2*img(32,2)+img(32,3)+2*(img(33,1)+2*img(33,2)+img(33,3))+img(34,1)+2*img(34,2)+img(34,3);
poke = mat2gray(img);
figure; imshow( poke );

% local1 = fopen('local_img_file_1.txt', 'r');
% loc1 = fscanf(local1, '%d', [size, 75]);
% fclose(local1);
% loc1 = transpose(loc1);
% loc1_out = mat2gray(loc1);
% %figure; imshow( loc1_out );
% 
% local2 = fopen('local_img_file_2.txt', 'r');
% loc2 = fscanf(local2, '%d', [size, 75]);
% fclose(local2);
% loc2 = transpose(loc2);
% loc2_out = mat2gray(loc2);
% %figure; imshow( loc2_out );
% 
% local3 = fopen('local_img_file_3.txt', 'r');
% loc3 = fscanf(local3, '%d', [size, 75]);
% fclose(local3);
% loc3 = transpose(loc3);
% loc3_out = mat2gray(loc3);
% %figure; imshow( loc3_out );


fid_out = fopen('smooth-mpi-v1-output.txt', 'r');
img_out = fscanf(fid_out, '%f', [size, size]);
fclose(fid_out);
img_out = transpose(img_out);
poke_out = mat2gray(img_out);
figure; imshow( poke_out );

w=img_out(33, 2)-z/16
