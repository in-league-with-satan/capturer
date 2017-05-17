#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

#ifdef USE_SDL2
#include <SDL2/SDL.h>

SDL_Rect sdl_rect;
SDL_Event sdl_event;

#endif


#include "frame_buffer.h"
#include "ff_tools.h"

#include "sdl2_video_output_thread.h"

Sdl2VideoOutpitThread::Sdl2VideoOutpitThread(QObject *parent) :
    QThread(parent)
{
    frame_buffer=new FrameBuffer(this);
    frame_buffer->setMaxSize(1);

    setTerminationEnabled();

#ifdef USE_SDL2

    av_frame=nullptr;
    av_frame_converted=nullptr;

    sws_ctx=nullptr;

    sdl_window=nullptr;
    sdl_renderer=nullptr;
    sdl_texture=nullptr;

    //

    int ret=SDL_Init(SDL_INIT_VIDEO);

    if(ret!=0) {
        qCritical() << "SDL_Init err";
        exit(1);
    }

    start(QThread::NormalPriority);

#endif

}

Sdl2VideoOutpitThread::~Sdl2VideoOutpitThread()
{
    terminate();
}

FrameBuffer *Sdl2VideoOutpitThread::frameBuffer()
{
    return frame_buffer;
}

void Sdl2VideoOutpitThread::run()
{
    while(true) {
        checkFrame();

        QCoreApplication::processEvents();

        usleep(1000);
    }
}

void Sdl2VideoOutpitThread::init()
{
#ifdef USE_SDL2

    sdl_rect.x=0;
    sdl_rect.y=0;
    sdl_rect.w=QApplication::desktop()->rect().width();
    sdl_rect.h=QApplication::desktop()->rect().height();

    //

    if(av_frame)
        av_free(av_frame);

    if(av_frame_converted) {
        av_free(av_frame_converted);
        av_frame_converted=nullptr;
    }

    if(sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx=nullptr;
    }

    if(sdl_window)
        SDL_DestroyWindow(sdl_window);

    if(sdl_renderer)
        SDL_DestroyRenderer(sdl_renderer);

    if(sdl_texture)
        SDL_DestroyTexture(sdl_texture);

    //

    sdl_window=SDL_CreateWindow("capturer_sdl",
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                sdl_rect.w, sdl_rect.h,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
                                );

    if(!sdl_window) {
        qCritical() << "SDL_CreateWindow err";
        exit(1);
    }

    sdl_renderer=SDL_CreateRenderer(sdl_window,
                                    -1,
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
                                    );

    if(!sdl_renderer) {
        qCritical() << "SDL_CreateRenderer err";
        exit(1);
    }

    sdl_texture=SDL_CreateTexture(sdl_renderer,
                                   SDL_PIXELFORMAT_ARGB8888
                                  // SDL_PIXELFORMAT_RGBA8888
                                  // SDL_PIXELFORMAT_ABGR8888
                                  // SDL_PIXELFORMAT_BGRA8888
                                  // SDL_PIXELFORMAT_ARGB2101010
                                  ,
                                  SDL_TEXTUREACCESS_STATIC,
                                  sdl_rect.w, sdl_rect.h
                                  );

    if(!sdl_texture) {
        qCritical() << "SDL_CreateTexture err";
        exit(1);
    }

    SDL_SetTextureBlendMode(sdl_texture, SDL_BLENDMODE_BLEND);


    // allocate and init a re-usable frame
    av_frame=alloc_frame(AV_PIX_FMT_BGRA, in_frame_size.width(), in_frame_size.height());

    // av_frame=new AVFrame();
    // av_frame->linesize[0]=in_frame_size.width()*4;

    if(!av_frame) {
        qCritical() << "could not allocate video frame";
        exit(1);
    }

    if(sdl_rect.w!=in_frame_size.width() || sdl_rect.h!=in_frame_size.height()) {
        av_frame_converted=alloc_frame(AV_PIX_FMT_BGRA, sdl_rect.w, sdl_rect.h);

        if(!av_frame_converted) {
            qCritical() << "could not allocate video frame";
            exit(1);
        }

        sws_ctx=sws_getContext(
                    in_frame_size.width(),
                    in_frame_size.height(),
                    AV_PIX_FMT_BGRA,
                    sdl_rect.w,
                    sdl_rect.h,
                    AV_PIX_FMT_BGRA,
                    // SWS_FAST_BILINEAR    // 20
                    // SWS_BILINEAR         // 23
                    // SWS_BICUBIC          // 24
                    // SWS_X                // 24
                    SWS_POINT            // 18
                    // SWS_AREA             // 23
                    // SWS_BICUBLIN         // 24
                    // SWS_GAUSS            // 22
                    // SWS_SINC             // 25
                    // SWS_LANCZOS          // 25
                    // SWS_SPLINE           // 23
                    ,
                    nullptr,
                    nullptr,
                    nullptr
                    );

        if(!sws_ctx) {
            qCritical() << "sws_getContext err";
            exit(1);
        }
    }

#endif
}

void Sdl2VideoOutpitThread::drawFrame(QByteArray *ba_frame)
{
#ifdef USE_SDL2

    AVFrame *frame;

    uint8_t *ptr_data_orig=av_frame->data[0];

//    byteArrayToAvFrame(ba_frame, av_frame);

    av_frame->data[0]=(uint8_t*)ba_frame->data();


    if(sdl_rect.w!=in_frame_size.width() || sdl_rect.h!=in_frame_size.height()) {
        sws_scale(sws_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, av_frame_converted->data, av_frame_converted->linesize);

        frame=av_frame_converted;

    } else {
        frame=av_frame;
    }

    int ret;

    ret=SDL_RenderClear(sdl_renderer);

    if(ret!=0) {
        qCritical() << "SDL_RenderClear err";
    }

    ret=SDL_UpdateTexture(sdl_texture, &sdl_rect, frame->data[0], frame->linesize[0]);

    if(ret!=0) {
        qCritical() << "SDL_UpdateTexture err";
    }

    ret=SDL_RenderCopy(sdl_renderer, sdl_texture, &sdl_rect, &sdl_rect);

    if(ret!=0) {
        qCritical() << "SDL_RenderCopy err";
    }

    SDL_RenderPresent(sdl_renderer);

    //

    av_frame->data[0]=ptr_data_orig;

#endif
}

void Sdl2VideoOutpitThread::drawFrameQImage(QByteArray *ba_frame)
{
#ifdef USE_SDL2

    int ret;

    ret=SDL_RenderClear(sdl_renderer);

    if(ret!=0) {
        qCritical() << "SDL_RenderClear err";
    }

    //

    if(sdl_rect.w!=in_frame_size.width() || sdl_rect.h!=in_frame_size.height()) {
        QImage img_src=QImage((uchar*)ba_frame->data(), in_frame_size.width(), in_frame_size.height(), QImage::Format_ARGB32);

        QImage img_scaled=img_src.scaled(sdl_rect.w, sdl_rect.h);

        if(img_scaled.isNull())
            return;

        ret=SDL_UpdateTexture(sdl_texture, &sdl_rect, img_scaled.bits(), img_scaled.width()*4);

    } else {
        ret=SDL_UpdateTexture(sdl_texture, &sdl_rect, ba_frame->data(), in_frame_size.width()*4);

    }

    if(ret!=0) {
        qCritical() << "SDL_UpdateTexture err";
    }

    ret=SDL_RenderCopy(sdl_renderer, sdl_texture, &sdl_rect, &sdl_rect);

    if(ret!=0) {
        qCritical() << "SDL_RenderCopy err";
    }

    SDL_RenderPresent(sdl_renderer);

#endif

}

void Sdl2VideoOutpitThread::checkFrame()
{
#ifdef USE_SDL2

    Frame::ptr frame;

    SDL_PollEvent(&sdl_event);


    if(frame_buffer->isEmpty())
        return;

    frame=frame_buffer->take();

    if(in_frame_size!=frame->video.decklink_frame.getSize()) {
        in_frame_size=frame->video.decklink_frame.getSize();

        init();
    }

    // drawFrame(&frame.ba_video);
    drawFrameQImage(frame->video.raw);

#endif
}
