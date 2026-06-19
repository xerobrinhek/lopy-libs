class Button {
    private int handle;
    private int type;

    public Button(String label, int x, int y, int w, int h) {
        this.type = 1;
        this.handle = cr.create_widget(this.type, label, x, y, w, h);
    }

    public int getHandle() { return this.handle; }

    public void onClick(func callback) {
        cr.button_on_click(this.handle, callback);
    }
}