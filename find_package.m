function [frame_data,dataLen] = find_package(filename,frame_header,frame_tail)
% 这个函数用于解析二进制文件中的帧数据
% 打开二进制文件
% 输入：filename，二进制文件名
% 输入：frame_header，帧头的十六进制字符串，eg:'0000AABB'
% 输入：frame_tail，帧尾的十六进制字符串,eg:'0000CCDD'
% 输出：frame_data，带帧头的数据

% filename = 'C:\Users\DELL\Desktop\MFC测试\MFC测试20231030\2023-10-31_20-41-17\00000CH1.dat'
% 打开二进制文件
fileID = fopen(filename,'r');

% 读取所有字节并转换为十六进制字符串
bytes = fread(fileID,'*uint8');
hexstr = dec2hex(bytes);
hexstr = reshape(hexstr.',[],1).';

% 关闭文件
fclose(fileID);

% 定义帧头和帧尾的十六进制字符串
% frame_header = '0000AABB';
% frame_tail = '0000CCDD';

%单个数据帧字节数
one_package_len = 512*4 + 4*4;
%包头包尾各自的字节数
head_len = 4;

% 查找帧头和帧尾在字符串中的位置
header_pos = strfind(hexstr,frame_header);
tail_pos = strfind(hexstr,frame_tail);

% 检查是否有相同数量的帧头和帧尾
if length(header_pos) ~= length(tail_pos)
	error('Invalid number of frame headers or tails');
end

% 获取帧个数
num_frames = length(header_pos);

%给出个数据帧的长度
dataLen = tail_pos - header_pos;

% 检查是否有不完整的帧
if any(tail_pos - header_pos ~= one_package_len*2-head_len*2)
	disp('Incomplete frame detected');
end

% 找出不完整的帧
skipFrame = (tail_pos - header_pos ~= one_package_len*2-head_len*2);
for i = 1:num_frames
	if skipFrame(i)
		fprintf('\tFrame No.%02d is incomplete!\r',i)
	end
end

frame_data = uint8(zeros(num_frames-sum(skipFrame),one_package_len));

% 循环遍历每一帧
frameInd = 1;
for i = 1:num_frames
    % 跳过不完整数据
    if (skipFrame(i))
        continue;
    else
        % 获取当前帧的十六进制字符串
        frame_str = hexstr((header_pos(i)):(tail_pos(i)-1+8));
        % 将十六进制字符串转换为字节数组
        frame_bytes = hex2dec(reshape(frame_str,2,[])');
        % 将当前帧保存,转置
        frame_data(frameInd,:) = frame_bytes.';
        % 下一帧
        frameInd = frameInd+1;
    end
end