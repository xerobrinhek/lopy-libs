class Window {
    private int handle;

    public Window(String title, int width, int height) {
        this.handle = cr.create_window(title, width, height);
    }

    public void setTitle(String title) {
        cr.set_window_title(this.handle, title);
    }

    public void clear(int r, int g, int b) {
        cr.clear_window(this.handle, r, g, b);
    }

    public void show() {
        cr.show_window(this.handle);
    }

    public int shouldClose() {
        return cr.window_should_close(this.handle);
    }

    public void pollEvents() {
        cr.poll_events(this.handle);
    }

    public void sleep(int ms) {
        cr.sleep_ms(ms);
    }

    public void destroy() {
        cr.destroy_window(this.handle);
    }
}