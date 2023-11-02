function [frame_data,dataLen] = find_package2(filename,frame_head,frame_tail,frame_size)
% 这个函数用于解析二进制文件中的帧数据
% 打开二进制文件
% 输入：filename，二进制文件名
% 输入：frame_head，帧头每个字节的十进制数值，eg:[0 0 170 187] 对应0x0000AABB
% 输入：frame_tail，帧尾的十六进制字符串,eg:[0 0 204 221]  对应0x0000CCDD
% 输入：frame_size，每一帧的字节数（包含帧头帧尾）
% 输出：frame_data，带帧头的数据
% 输出：dataLen，各数据帧的长度(含包头包尾本身)

% 打开二进制文件并读取其中的数据。
fid = fopen(filename, 'rb');
data = fread(fid, Inf, 'uint8');
fclose(fid);

%找到帧头和帧尾的位置
% frame_size = 512*4 + 4*4; % 每一帧的大小
% frame_head = [0 0 170 187];
% frame_tail = [0 0 204 221];
head_size = length(frame_head);
 
head_positions = find(data == frame_head(1) & circshift(data, -1) == frame_head(2)...
    & circshift(data, -2) == frame_head(3) & circshift(data, -3) == frame_head(4));
tail_positions = find(data == frame_tail(1) & circshift(data, -1) == frame_tail(2)... 
    & circshift(data, -2) == frame_tail(3) & circshift(data, -3) == frame_tail(4));

% 获取帧个数
num_frames = length(head_positions);

%给出个数据帧的长度
dataLen = tail_positions - head_positions + head_size;

% 检查是否有不完整的帧
if any(tail_positions - head_positions ~= frame_size - head_size)
	disp('Incomplete frame detected');
end

% 找出不完整的帧
skipFrame = (tail_positions - head_positions ~= frame_size - head_size);
for i = 1:num_frames
	if skipFrame(i)
		fprintf('\tFrame No.%02d is incomplete!\r',i)
	end
end

%根据帧头和帧尾的位置来解析每一帧的数据,初值为-1
frame_data = -uint8(ones(num_frames-sum(skipFrame),frame_size));

% 循环遍历每一帧
for i = 1:length(head_positions)
     % 跳过不完整数据
    if (skipFrame(i))
        continue;
    else
        % 将当前帧转置保存
        one_frame = data(head_positions(i):tail_positions(i) + head_size - 1);
        frame_data(i,:) = one_frame.';
    end
end