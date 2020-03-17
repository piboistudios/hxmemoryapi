package mem;

class MapView {
    public function write(bytes:haxe.io.Bytes):Bool {
        throw 'not implemented';
    }
    public function persist() {
        throw 'not implemented';
    }
    public function read():haxe.io.Bytes {
        throw 'not implemented';
    }
}