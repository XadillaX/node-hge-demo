#include <hge.h>
#include <nan.h>

namespace node_hge {
    HGE* hge = 0;
    Nan::Callback* node_frame_callback;
    Nan::Callback* node_render_callback;
    Nan::Callback* hge_done_callback;
    uv_cond_t cond;
    uv_cond_t cond_render;
    uv_mutex_t mutex;
    uv_mutex_t mutex_render;

    void JSFrame(uv_async_t* handle)
    {
        Nan::HandleScope scope;
        float* delta_time = (float*)handle->data;
        v8::Local<v8::Number> dt = Nan::New<v8::Number>(*delta_time);
        delete delta_time;
        v8::Local<v8::Value> args[] = { dt };
        node_frame_callback->Call(1, args);
        uv_mutex_lock(&mutex);
        uv_cond_signal(&cond);
        uv_mutex_unlock(&mutex);
    }

    void JSRender(uv_async_t* handle)
    {
        Nan::HandleScope scope;
        node_render_callback->Call(0, NULL);
        uv_mutex_lock(&mutex_render);
        uv_cond_signal(&cond_render);
        uv_mutex_unlock(&mutex_render);
    }

    void OnHGEDone(uv_async_t* handle)
    {
        Nan::HandleScope scope;
        hge_done_callback->Call(0, NULL);
        delete handle;
    }

    bool RenderFunc()
    {
        uv_async_t async;
        uv_async_init(uv_default_loop(), &async, JSRender);
        hge->Gfx_BeginScene();
        uv_mutex_lock(&mutex_render);
        uv_async_send(&async);

        uv_cond_wait(&cond_render, &mutex_render);
        uv_mutex_unlock(&mutex_render);
        hge->Gfx_EndScene();
        return false;
    }

    bool FrameFunc()
    {
        uv_async_t async;
        uv_async_init(uv_default_loop(), &async, JSFrame);
        float* delta_time = new float(hge->Timer_GetDelta());
        async.data = (void*)delta_time;
        uv_mutex_lock(&mutex);
        uv_async_send(&async);
        uv_cond_wait(&cond, &mutex);
        uv_mutex_unlock(&mutex);
        return false;
    }

    void GameThreadFunc(void*)
    {
        hge->System_Initiate();
        hge->System_Start();
        hge->System_Shutdown();
        hge->Release();
        printf("done\n");
        uv_async_t* async = new uv_async_t();
        uv_async_init(uv_default_loop(), async, OnHGEDone);
        uv_async_send(async);
    }

    NAN_METHOD(SetFrameCallback)
    {
        node_frame_callback = new Nan::Callback(info[0].As<v8::Function>());
    }

    NAN_METHOD(SetRenderCallback)
    {
        node_render_callback = new Nan::Callback(info[0].As<v8::Function>());
    }

    NAN_METHOD(SetHGEDoneCallback)
    {
        hge_done_callback = new Nan::Callback(info[0].As<v8::Function>());
    }

    NAN_METHOD(Start)
    {
        hge = hgeCreate(HGE_VERSION);
        hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
        hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
        hge->System_SetState(HGE_WINDOWED, true);
        hge->System_SetState(HGE_USESOUND, false);
        hge->System_SetState(HGE_TITLE, "HGE Tutorial 01 - Minimal HGE application");
        hge->System_SetState(HGE_FPS, HGEFPS_VSYNC);
        uv_thread_t handle;
        uv_thread_create(&handle, GameThreadFunc, NULL);
    }

    NAN_METHOD(GfxClear)
    {
        uint32_t color = Nan::To<uint32_t>(info[0]).FromJust();
        hge->Gfx_Clear(color);
    }

    NAN_MODULE_INIT(Init)
    {
        uv_cond_init(&cond);
        uv_cond_init(&cond_render);
        uv_mutex_init(&mutex);
        uv_mutex_init(&mutex_render);
        Nan::Set(target, Nan::New("setFrameCallback").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(SetFrameCallback)).ToLocalChecked());
        Nan::Set(target, Nan::New("setRenderCallback").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(SetRenderCallback)).ToLocalChecked());
        Nan::Set(target, Nan::New("setHGEDoneCallback").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(SetHGEDoneCallback)).ToLocalChecked());
        Nan::Set(target, Nan::New("start").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Start)).ToLocalChecked());
        Nan::Set(target, Nan::New("gfxClear").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GfxClear)).ToLocalChecked());
    }
}

NODE_MODULE(node_hge, node_hge::Init)