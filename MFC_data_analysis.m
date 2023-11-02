%% write by maoxiaoqing 2023/6/20—23:52:01
%查看是否丢包，根据绘制的图形，看Z值存在-1的位置
clc;clear;
close all;

%% 设置参数
PV = 2028; % 炮号
CHX = 0; %想要绘图的某个通道编号，如果为0，则表示绘制所有通道编号
ENERGY = [1,512]; %求和的能量道址范围（绘图用到）
count_rate_curve = 1; %显示：1；不显示：0;计数率曲线显示标志
NUMBER = 2; %信道数，CH1，CH2，CH3，CH4
ChannelNum = 16; %每个信道的探测器数目
DataHead=43707; % 数据包头 0x0000AABB
DataTail=52445; % 数据包尾 0x0000CCDD

%% 一、交互式获取文件路径
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

%% 二、读取二进制文件，直接转化为数据
% 每4个字节为一个数据，也就是uint32。32位为一个数字。0x0000AABB为一个数
data_init = cell(NUMBER,1);
for ch = 1:NUMBER
    fileID = fopen(whole_file_path{ch},'r');
    if(fileID >3 )
        data_init{ch} = fread(fileID,'uint32','b');%uint32 uint8
        fclose(fileID);
    end
end

% temp = data{1,1};temp = reshape(temp,516,[]);

%% 参数解析（能谱形式(512道能谱或者16道能谱)、刷新时间）
% 直接拿1号通道解析
ch = 1;
% 能谱形式
head_pos = find(data_init{ch} == DataHead); 
d_head = diff(head_pos); %包头作差，判断包长，确定能谱形式
analysis = tabulate(d_head); %统计频次
[Y_col,Ind_row]= max(analysis(:,2)); % 最大值及行号
data_len = analysis(Ind_row,1); %总数据包长度，包含包头、包尾、时间信息、探测器编号4个数

% 刷新时间
time_pos = data_init{ch}(head_pos+1:data_len:end);
d_tmp = diff(time_pos);%每个包时间信息作差，判断刷新时间
analysis2 = tabulate(d_tmp);
% 排序后挑选次大值及行号索引
analysis_new = sort(analysis2(:,2));
Ind_row = find(analysis2(:,2) == analysis_new(end-1));
refresh_time = analysis2(Ind_row,1);

%% 三按时间顺序对各个通道数据进行排序
% temp = data{1,1};
% temp = reshape(temp,data_len,[]);
Data_All = cell(4,1);
for ch = 1:NUMBER
    head_pos = find(data_init{ch} == DataHead);
    d_head = diff(head_pos);
    single_data_num = length(d_head);
    Data_One = -1*ones(data_len-4,ChannelNum,length(head_pos)); %单个通道的数据,初值为-1
    maxTimer = 0;
    for i = 1:single_data_num
        % 判断数据包的长度，查找完整包，并将其放入数组
        if(d_head(i) == data_len) 
            single_data = data_init{ch,1}(head_pos(i)+3:head_pos(i)+data_len-2);
            time = data_init{ch,1}(head_pos(i)+1);
            timer = time/refresh_time; % 给出第多少个点
            if maxTimer < timer
                maxTimer = timer;
            end
            energy_ch = log2(data_init{ch,1}(head_pos(i)+2))+1; %能道
            Data_One(:, energy_ch,timer) = single_data;
        end
    end
    Data_All{ch} = Data_One(:,:,1:maxTimer);
end

%% 给出各个通道最小完整数据包数目
data_num = zeros(NUMBER,1);
for ch = 1:NUMBER
    data_num(ch) = length(Data_All{ch}(1,1,:));
end
data_len_min = min(data_num);

%% 汇总各个信道的数据,拼接四个信道的数据包
Data_merge = -1*ones(data_len-4,ChannelNum*NUMBER,data_len_min);

for data_id = 1:data_len_min
    for ch = 1:NUMBER
        for channel_id = 1:ChannelNum
            num = (ch-1)*ChannelNum + channel_id;
            Data_merge(:,num,data_id) = Data_All{ch}(:,channel_id,data_id);
        end
    end
end

%% 绘图分析
if(CHX ~= 0 )
    channel_num = data_len - 4;
    x = refresh_time:refresh_time:data_len_min*refresh_time;%时间
    y = 1:1:channel_num;%道址
    [X,Y] = meshgrid(x,y);
    Z = Data_merge(:,CHX,:);%某通道数据
    Z = Z(1:channel_num,1,:);
    Z = reshape(Z,channel_num,data_len_min);
    figure,mesh(X,Y,Z);
    xlabel('Time(ms)');
    ylabel('Channel');
    zlabel('Count');
    title(['炮号为',num2str(PV),'的Detector.NO ',num2str(CHX),'的所有能量测试结果']);
elseif(sum(ENERGY) ~=0)
    CH_NUM = NUMBER*ChannelNum-1;
    x = refresh_time:refresh_time:data_len_min*refresh_time;%时间轴
    y = 1:1:CH_NUM;%通道号
    [X,Y] = meshgrid(x,y);

    Z_tmp = Data_merge(ENERGY(1):ENERGY(2),:,:);%保存某个道址区间下的数据
    Z_tmp(:,12,:) = [];
    Z = sum(Z_tmp(:,:,:),1);
    Z = reshape(Z,CH_NUM,data_len_min);
    Z(find(Z<0)) = -1;
    figure,mesh(X,Y,Z);
    xlabel('Time(ms)');
    ylabel('Detector NO.');
    zlabel('Count');
    title(['炮号为',num2str(PV),'的 ',num2str(ENERGY),'能量道址下的测试结果'])
end





