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
    public function write(bytes:haxe.io.Bytes, offset:haxe.Int64):Bool {
        trace('bytes: ${bytes.length}');
        final ret = MemLib.write_to_view( viewHandle, offset.high, offset.low, bytes);
        checkErrors(viewHandle);
        return ret;
    }
    public function read(offset:haxe.Int64):haxe.io.Bytes {
        final ret = MemLib.read_view(viewHandle, offset.high, offset.low);
        checkErrors(viewHandle);
        return ret;
    }
    public function persist() {
        final ret = viewHandle.persist_view();
        checkErrors(viewHandle);
        return ret;
    }
    
}