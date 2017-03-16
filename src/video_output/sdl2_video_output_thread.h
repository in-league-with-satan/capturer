#ifndef SDL2_VIDEO_OUTPUT_THREAD_H
#define SDL2_VIDEO_OUTPUT_THREAD_H

#include <QThread>
#include <QSize>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

struct SwsContext;
struct AVFrame;

class FrameBuffer;

class Sdl2VideoOutpitThread : public QThread
{
    Q_OBJECT

public:
    explicit Sdl2VideoOutpitThread(QObject *parent=0);
    virtual ~Sdl2VideoOutpitThread();

    FrameBuffer *frameBuffer();

protected:
    void run();

    void init();
    void drawFrame(QByteArray *ba_frame);
    void drawFrameQImage(QByteArray *ba_frame);

private slots:
    void checkFrame();

private:
    FrameBuffer *frame_buffer;

    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture *sdl_texture;

    SwsContext *sws_ctx;

    QSize in_frame_size;

    AVFrame *av_frame;
    AVFrame *av_frame_converted;
};

#endif // SDL2_VIDEO_OUTPUT_THREAD_H
