/*
 * @file 	videoctl.h
 * @date 	2018/01/07 10:48
 *
 * @author 	itisyang
 * @Contact	itisyang@gmail.com
 *
 * @brief 	视频控制类
 * @note
 */
#ifndef VIDEOCTL_H
#define VIDEOCTL_H

#include <QObject>
#include <QThread>
#include <QString>

#include "globalhelper.h"
#include "datactl.h"

// 视频控制类，负责视频的播放、暂停、停止、音量控制等基本操作
// 采用单例模式，确保全局只有一个实例
class VideoCtl : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例
    static VideoCtl* GetInstance();
    ~VideoCtl();

    /**
     * @brief 开始播放视频
     *
     * @param strFileName 文件完整路径
     * @param widPlayWid 播放窗口ID
     * @return true 成功，false 失败
     */
    bool StartPlay(QString strFileName, WId widPlayWid);

    /**
     * @brief 音频解码函数，用于解码音频帧
     *
     * @param is 视频状态结构体
     * @return 解码后的音频样本数量
     */
    int audio_decode_frame(VideoState *is);

    /**
     * @brief 更新样本显示，用于音频波形显示
     *
     * @param is 视频状态结构体
     * @param samples 音频样本数据
     * @param samples_size 样本数据大小
     */
    void update_sample_display(VideoState *is, short *samples, int samples_size);

    /**
     * @brief 设置时钟，用于同步音频和视频
     *
     * @param c 时钟结构体
     * @param pts 时间戳
     * @param serial 序列号
     * @param time 当前时间
     */
    void set_clock_at(Clock *c, double pts, int serial, double time);

    /**
     * @brief 同步时钟到从时钟
     *
     * @param c 主时钟
     * @param slave 从时钟
     */
    void sync_clock_to_slave(Clock *c, Clock *slave);

signals:
    // 发送错误信息
    void SigPlayMsg(QString strMsg);

    // 视频宽高发生变化
    void SigFrameDimensionsChanged(int nFrameWidth, int nFrameHeight);

    // 视频总时长
    void SigVideoTotalSeconds(int nSeconds);

    // 视频播放时长
    void SigVideoPlaySeconds(int nSeconds);

    // 视频音量变化
    void SigVideoVolume(double dPercent);

    // 播放暂停状态
    void SigPauseStat(bool bPaused);

    // 停止播放
    void SigStop();

    // 停止播放完成
    void SigStopFinished();

    // 开始播放
    void SigStartPlay(QString strFileName);

public slots:
    // 播放进度调整
    void OnPlaySeek(double dPercent);

    // 音量调整
    void OnPlayVolume(double dPercent);

    // 向前快进
    void OnSeekForward();

    // 向后快退
    void OnSeekBack();

    // 增加音量
    void OnAddVolume();

    // 减小音量
    void OnSubVolume();

    // 暂停播放
    void OnPause();

    // 停止播放
    void OnStop();

private:
    // 构造函数，私有化防止外部直接构造
    explicit VideoCtl(QObject *parent = nullptr);

    /**
     * @brief 初始化视频控制类
     *
     * @return true 初始化成功，false 初始化失败
     */
    bool Init();

    /**
     * @brief 连接信号和槽
     *
     * @return true 连接成功，false 连接失败
     */
    bool ConnectSignalSlots();

    /**
     * @brief 从视频队列中获取数据，并解码数据，得到可显示的视频帧
     *
     * @param is 视频状态结构体
     * @param frame 视频帧
     * @return 解码成功返回1，失败返回0
     */
    int get_video_frame(VideoState *is, AVFrame *frame);

    /**
     * @brief 音频解码线程
     *
     * @param arg 视频状态结构体
     * @return 0 表示成功，负值表示错误
     */
    int audio_thread(void *arg);

    /**
     * @brief 视频解码线程
     *
     * @param arg 视频状态结构体
     * @return 0 表示成功，负值表示错误
     */
    int video_thread(void *arg);

    /**
     * @brief 字幕解码线程
     *
     * @param arg 视频状态结构体
     * @return 0 表示成功，负值表示错误
     */
    int subtitle_thread(void *arg);

    /**
     * @brief 同步音频数据
     *
     * @param is 视频状态结构体
     * @param nb_samples 音频样本数量
     * @return 调整后的音频样本数量
     */
    int synchronize_audio(VideoState *is, int nb_samples);

    /**
     * @brief 打开音频设备
     *
     * @param opaque 用户数据
     * @param wanted_channel_layout 期望的声道布局
     * @param wanted_sample_rate 期望的采样率
     * @param audio_hw_params 音频硬件参数
     * @return 音频缓冲区大小
     */
    int audio_open(void* opaque, AVChannelLayout* wanted_channel_layout, int wanted_sample_rate, struct AudioParams* audio_hw_params);

    /**
     * @brief 打开流
     *
     * @param is 视频状态结构体
     * @param stream_index 流索引
     * @return 0 表示成功，负值表示错误
     */
    int stream_component_open(VideoState *is, int stream_index);

    /**
     * @brief 检查流是否有足够的数据包
     *
     * @param st 流结构体
     * @param stream_id 流ID
     * @param queue 数据包队列
     * @return true 表示有足够的数据包，false 表示没有
     */
    int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue);

    /**
     * @brief 检查是否是实时流
     *
     * @param s 格式上下文
     * @return true 表示是实时流，false 表示不是
     */
    int is_realtime(AVFormatContext *s);

    /**
     * @brief 读取线程
     *
     * @param CurStream 当前播放流
     */
    void ReadThread(VideoState *CurStream);

    /**
     * @brief 播放控制循环
     *
     * @param CurStream 当前播放流
     */
    void LoopThread(VideoState *CurStream);

    /**
     * @brief 打开流
     *
     * @param filename 文件名
     * @return 视频状态结构体
     */
    VideoState *stream_open(const char *filename);

    /**
     * @brief 切换流通道
     *
     * @param is 视频状态结构体
     * @param codec_type 媒体类型（音频、视频、字幕）
     */
    void stream_cycle_channel(VideoState *is, int codec_type);

    /**
     * @brief 刷新循环等待事件
     *
     * @param is 视频状态结构体
     * @param event SDL事件
     */
    void refresh_loop_wait_event(VideoState *is, SDL_Event *event);

    /**
     * @brief 章节跳转
     *
     * @param is 视频状态结构体
     * @param incr 增量（正数表示向前跳转，负数表示向后跳转）
     */
    void seek_chapter(VideoState *is, int incr);

    /**
     * @brief 刷新视频显示
     *
     * @param opaque 用户数据
     * @param remaining_time 剩余时间
     */
    void video_refresh(void *opaque, double *remaining_time);

    /**
     * @brief 将视频帧加入队列
     *
     * @param is 视频状态结构体
     * @param src_frame 源视频帧
     * @param pts 时间戳
     * @param duration 持续时间
     * @param pos 位置
     * @param serial 序列号
     * @return 0 表示成功，负值表示错误
     */
    int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial);

    /**
     * @brief 更新音量
     *
     * @param sign 符号（1 表示增加，-1 表示减少）
     * @param step 步长
     */
    void UpdateVolume(int sign, double step);

    /**
     * @brief 显示视频画面
     *
     * @param is 视频状态结构体
     */
    void video_display(VideoState *is);

    /**
     * @brief 打开视频窗口
     *
     * @param is 视频状态结构体
     * @return 0 表示成功，负值表示错误
     */
    int video_open(VideoState *is);

    /**
     * @brief 退出播放
     *
     * @param is 视频状态结构体
     */
    void do_exit(VideoState* &is);

    /**
     * @brief 获取时钟值
     *
     * @param c 时钟结构体
     * @return 当前时钟值
     */
    double get_clock(Clock *c);

    /**
     * @brief 设置时钟
     *
     * @param c 时钟结构体
     * @param pts 时间戳
     * @param serial 序列号
     */
    void set_clock(Clock *c, double pts, int serial);

    /**
     * @brief 设置时钟速度
     *
     * @param c 时钟结构体
     * @param speed 速度
     */
    void set_clock_speed(Clock *c, double speed);

    /**
     * @brief 初始化时钟
     *
     * @param c 时钟结构体
     * @param queue_serial 队列序列号
     */
    void init_clock(Clock *c, int *queue_serial);

    /**
     * @brief 获取主同步时钟类型
     *
     * @param is 视频状态结构体
     * @return 主同步时钟类型
     */
    int get_master_sync_type(VideoState *is);

    /**
     * @brief 获取主时钟值
     *
     * @param is 视频状态结构体
     * @return 主时钟值
     */
    double get_master_clock(VideoState *is);

    /**
     * @brief 检查外部时钟速度
     *
     * @param is 视频状态结构体
     */
    void check_external_clock_speed(VideoState *is);

    /**
     * @brief 流跳转
     *
     * @param is 视频状态结构体
     * @param pos 位置
     * @param rel 相对位置
     */
    void stream_seek(VideoState *is, int64_t pos, int64_t rel);

    /**
     * @brief 切换暂停状态
     *
     * @param is 视频状态结构体
     */
    void stream_toggle_pause(VideoState *is);

    /**
     * @brief 暂停播放
     *
     * @param is 视频状态结构体
     */
    void toggle_pause(VideoState *is);

    /**
     * @brief 跳转到下一帧
     *
     * @param is 视频状态结构体
     */
    void step_to_next_frame(VideoState *is);

    /**
     * @brief 计算目标延迟
     *
     * @param delay 延迟时间
     * @param is 视频状态结构体
     * @return 目标延迟时间
     */
    double compute_target_delay(double delay, VideoState *is);

    /**
     * @brief 计算视频帧持续时间
     *
     * @param is 视频状态结构体
     * @param vp 当前视频帧
     * @param nextvp 下一视频帧
     * @return 视频帧持续时间
     */
    double vp_duration(VideoState *is, Frame *vp, Frame *nextvp);

    /**
     * @brief 更新视频PTS
     *
     * @param is 视频状态结构体
     * @param pts 时间戳
     * @param pos 位置
     * @param serial 序列号
     */
    void update_video_pts(VideoState *is, double pts, int64_t pos, int serial);

    /**
     * @brief 重新分配纹理
     *
     * @param texture 纹理
     * @param new_format 新格式
     * @param new_width 新宽度
     * @param new_height 新高度
     * @param blendmode 混合模式
     * @param init_texture 是否初始化纹理
     * @return 0 表示成功，负值表示错误
     */
    int realloc_texture(SDL_Texture **texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture);

    /**
     * @brief 计算显示矩形
     *
     * @param rect 矩形
     * @param scr_xleft 屏幕左上角X坐标
     * @param scr_ytop 屏幕左上角Y坐标
     * @param scr_width 屏幕宽度
     * @param scr_height 屏幕高度
     * @param pic_width 图片宽度
     * @param pic_height 图片高度
     * @param pic_sar 图片宽高比
     */
    void calculate_display_rect(SDL_Rect *rect, int scr_xleft, int scr_ytop, int scr_width, int scr_height, int pic_width, int pic_height, AVRational pic_sar);

    /**
     * @brief 上传纹理
     *
     * @param tex 纹理
     * @param frame 视频帧
     * @param img_convert_ctx 图像转换上下文
     * @return 0 表示成功，负值表示错误
     */
    int upload_texture(SDL_Texture *tex, AVFrame *frame, struct SwsContext **img_convert_ctx);

    /**
     * @brief 显示视频画面
     *
     * @param is 视频状态结构体
     */
    void video_image_display(VideoState *is);

    /**
     * @brief 关闭流对应的解码器等
     *
     * @param is 视频状态结构体
     * @param stream_index 流索引
     */
    void stream_component_close(VideoState *is, int stream_index);

    /**
     * @brief 关闭流
     *
     * @param is 视频状态结构体
     */
    void stream_close(VideoState *is);

private:
    static VideoCtl* m_pInstance; //< 单例指针

    bool m_bInited; //< 初始化标志
    bool m_bPlayLoop; //< 刷新循环标志

    VideoState* m_CurStream; //< 当前播放流状态

    SDL_Window *window; //< SDL窗口
    SDL_Renderer *renderer; //< SDL渲染器
    SDL_RendererInfo renderer_info = { 0 }; //< 渲染器信息
    SDL_AudioDeviceID audio_dev; //< 音频设备ID
    WId play_wid; //< 播放窗口ID

    int screen_width; //< 屏幕宽度
    int screen_height; //< 屏幕高度
    int startup_volume; //< 初始音量

    // 播放刷新循环线程
    std::thread m_tPlayLoopThread;

    int m_nFrameW; //< 当前视频帧宽度
    int m_nFrameH; //< 当前视频帧高度
};

#endif // VIDEOCTL_H
