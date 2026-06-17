#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE

#include <GL/gl.h>
#include "rglfw.c"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static GLFWwindow* window = NULL;
static float clear_r = 0.0f, clear_g = 0.0f, clear_b = 0.0f;
static int need_redraw = 1;
static pthread_t render_thread;
static int thread_running = 1;
static int window_ready = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void redraw_window() {
    if (!window) return;
    glClearColor(clear_r, clear_g, clear_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    need_redraw = 0;
}

static void framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
    need_redraw = 1;
    pthread_cond_signal(&cond);
}

__int128 create_window(const char* title, __int128 width, __int128 height) {
    glfwInitHint(GLFW_PLATFORM, GLFW_ANY_PLATFORM);
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return (__int128)(long long)window;
}

void destroy_window(__int128 handle) {
    thread_running = 0;
    pthread_cond_signal(&cond);
    if (window) {
        glfwDestroyWindow(window);
        window = NULL;
        glfwTerminate();
    }
}

__int128 window_should_close(__int128 handle) {
    if (!window) return 1;
    return glfwWindowShouldClose(window) ? 1 : 0;
}

void poll_events(__int128 handle) {
    if (!window) return;
    glfwPollEvents();
}

void set_window_title(__int128 handle, const char* title) {
    if (window) glfwSetWindowTitle(window, title);
}

void clear_window(__int128 handle, __int128 r, __int128 g, __int128 b) {
    if (!window) return;
    pthread_mutex_lock(&mutex);
    clear_r = (float)r / 255.0f;
    clear_g = (float)g / 255.0f;
    clear_b = (float)b / 255.0f;
    need_redraw = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void sleep_ms(__int128 ms) {
    struct timespec ts;
    ts.tv_sec = (long long)ms / 1000;
    ts.tv_nsec = ((long long)ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void* window_run(void* arg) {
    int handle = (int)(long long)arg;

    if (window) {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        window_ready = 1;
    }

    if (window) {
        glfwShowWindow(window);

        for (int i = 0; i < 3; i++) {
            glClearColor(clear_r, clear_g, clear_b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
        }
        need_redraw = 0;
    }

    while (thread_running && !window_should_close(handle)) {
        pthread_mutex_lock(&mutex);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 16000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        pthread_cond_timedwait(&cond, &mutex, &ts);

        if (window) {
            glfwPollEvents();
        }

        if (need_redraw && window) {
            pthread_mutex_unlock(&mutex);
            redraw_window();
            pthread_mutex_lock(&mutex);
        }

        pthread_mutex_unlock(&mutex);
    }

    destroy_window(handle);
    exit(0);
    return NULL;
}

void show_window(__int128 handle) {
    pthread_create(&render_thread, NULL, window_run, (void*)(long long)handle);
    pthread_detach(render_thread);

    while (!window_ready) {
        usleep(1000);
    }
}