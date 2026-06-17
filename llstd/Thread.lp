class Thread {
    private int handle;

    public Thread(Window window, string code) {
        this.handle = cr.thread_create(window, code);
    }

    public void start() {
        cr.thread_start(this.handle);
    }

    public void join() {
        cr.thread_join(this.handle);
    }

    public int getHandle() {
        return this.handle;
    }
}