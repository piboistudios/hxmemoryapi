package mem;
import mem.MemLib;
using mem.ErrorTools;
class MapView {
    var viewHandle:LibMapView;
    public var offset(default, null):haxe.Int64;
    public var size(default, null):UInt;
    function new(offset, size, viewHandle) {
        this.offset = offset;
        this.size = size;
        this.viewHandle = viewHandle;
    }
    static function checkErrors(viewHandle:LibMapView) {
        if(viewHandle.mapview_errored())
            throw viewHandle.mapview_get_errors().asErrorMsg();
    }
    public function write(bytes:haxe.io.Bytes):Bool {
        final ret = MemLib.write_to_view(viewHandle, bytes);
        checkErrors(viewHandle);
        return ret;
    }
    public function read():haxe.io.Bytes {
        final ret = MemLib.read_view(viewHandle);
        checkErrors(viewHandle);
        return ret;
    }
    public function persist() {
        final ret = viewHandle.persist_view();
        checkErrors(viewHandle);
        return ret;
    }
}