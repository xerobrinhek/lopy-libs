#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE

#include <GL/gl.h>
#include "rglfw.c"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static GLFWwindow* window = NULL;
static float clear_r = 0.0f, clear_g = 0.0f, clear_b = 0.0f;
static int need_redraw = 1;
static pthread_t render_thread;
static int thread_running = 1;
static int window_ready = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int last_button_state = GLFW_RELEASE;

// === ВИДЖЕТЫ ===
typedef struct Widget {
    int type;
    float x, y, w, h;
    char label[256];
    void* lambda_ptr;
    void (*lambda_func)(void*);
} Widget;

static Widget* widgets[100];
static int widget_count = 0;

// === ОКНА ===
typedef struct LopyWindow {
    GLFWwindow* glfw_window;
    Widget* window_widgets[100];
    int widget_count;
} LopyWindow;

static LopyWindow windows[10];
static int window_count = 0;

// === КОЛБЭК РЕСАЙЗА (объявлен ДО create_window) ===
static void framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
    need_redraw = 1;
    pthread_cond_signal(&cond);
}

__int128 create_widget(__int128 type, const char* label, __int128 x, __int128 y, __int128 w, __int128 h) {
    Widget* widget = malloc(sizeof(Widget));
    widget->type = (int)type;           // <-- приводим к int
    widget->x = (float)(int)x;          // <-- приводим к int, потом к float
    widget->y = (float)(int)y;
    widget->w = (float)(int)w;
    widget->h = (float)(int)h;
    strncpy(widget->label, label, 255);
    widget->label[255] = '\0';
    widget->lambda_ptr = NULL;
    widget->lambda_func = NULL;
    widgets[widget_count++] = widget;
    return (__int128)(long long)widget;
}

void draw_button(Widget* w) {
    if (!w) return;
    glColor3f(0.2f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(w->x, w->y);
    glVertex2f(w->x + w->w, w->y);
    glVertex2f(w->x + w->w, w->y + w->h);
    glVertex2f(w->x, w->y + w->h);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(w->x, w->y);
    glVertex2f(w->x + w->w, w->y);
    glVertex2f(w->x + w->w, w->y + w->h);
    glVertex2f(w->x, w->y + w->h);
    glEnd();
}

void button_on_click(__int128 widget_handle, void* lambda_ptr, void (*lambda_func)(void*)) {
    Widget* w = (Widget*)(long long)widget_handle;
    if (w->type == 1) {
        w->lambda_ptr = lambda_ptr;
        w->lambda_func = lambda_func;
    }
}

void draw_widgets(GLFWwindow* win) {
    for (int i = 0; i < widget_count; i++) {
        Widget* w = widgets[i];
        switch (w->type) {
            case 1: draw_button(w); break;
        }
    }
}

// === ФУНКЦИИ ОКОН ===
__int128 create_window(const char* title, __int128 width, __int128 height) {
    glfwInitHint(GLFW_PLATFORM, GLFW_ANY_PLATFORM);
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    LopyWindow* win = &windows[window_count++];
    win->glfw_window = window;
    win->widget_count = 0;

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return (__int128)(long long)win;
}

void add_widget(__int128 window_handle, __int128 widget_handle) {
    LopyWindow* win = (LopyWindow*)(long long)window_handle;
    Widget* w = (Widget*)(long long)widget_handle;
    if (win->widget_count < 100) {
        win->window_widgets[win->widget_count++] = w;
    }
}

// === ОСТАЛЬНЫЕ ФУНКЦИИ ===
static void redraw_window() {
    if (!window) return;
    glClearColor(clear_r, clear_g, clear_b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_widgets(window);
    glfwSwapBuffers(window);
    need_redraw = 0;
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

void handle_mouse_click(GLFWwindow* win) {
    int current_state = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);

    if (last_button_state == GLFW_RELEASE && current_state == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(win, &x, &y);

        for (int i = 0; i < widget_count; i++) {
            Widget* w = widgets[i];
            if (w->type == 1) {
                if (x >= w->x && x <= w->x + w->w &&
                    y >= w->y && y <= w->y + w->h) {
                    if (w->lambda_func) {
                        w->lambda_func(w->lambda_ptr);
                    }
                }
            }
        }
    }

    last_button_state = current_state;
}

void poll_events(__int128 handle) {
    if (!window) return;
    glfwPollEvents();
    handle_mouse_click(window);
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

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        window_ready = 1;
    }

    if (window) {
        glfwShowWindow(window);

        for (int i = 0; i < 3; i++) {
            glClearColor(clear_r, clear_g, clear_b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            draw_widgets(window);
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
            poll_events(handle);
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