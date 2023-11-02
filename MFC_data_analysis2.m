%% write by maoxiaoqing 2023/10/31—19:52:01
% 用于分析二进制文件中，包头包尾之间的长度是否一致，以及给出完整数据包的个数
clc;clear;
close all;

%% 一、设置参数
NUMBER = 2; %信道数，CH1，CH2，CH3，CH4
ChannelNum = 16; %每个信道的探测器数目
frame_size = 512*4 + 4*4; % 每一帧的大小
frame_header = [0 0 170 187];
frame_tail = [0 0 204 221];

%% 二、交互式获取文件路径
pathName = uigetdir;
fileName = dir(pathName);
whole_file_path = cell(NUMBER,1); %4个通道

for ch = 1:NUMBER
    for i=1:length(fileName)
        postfix = strcat('CH',num2str(ch),'.dat'); 
        if(contains(fileName(i).name,postfix)) % 查找后缀是否为CHn.dat，
            whole_file_path{ch} = fullfile(pathName,fileName(i).name);
        end
    end
end

%% 三、读取二进制文件，直接转化为数据
data_init = cell(NUMBER,1);
package = cell(NUMBER,1);
%各数据包长度（含包头包尾）
dataLen = cell(NUMBER,1);
for ch = 1:NUMBER
    [package{ch},dataLen{ch}] = find_package2(whole_file_path{ch}, frame_header, frame_tail,frame_size);
end

%观察各个数据包长度是否一致，直接观察最大最小值是否一致，
DataLen1 = dataLen{1};
DataLen2 = dataLen{2};
%观察各个数据包拼接起来的数据是否合理
dataAll_1 = package{1};
dataAll_2 = package{2};










