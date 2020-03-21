package mem;
import ammer.*;
import ammer.ffi.*;

@:ammer.nativePrefix("mem_")
class MemLib extends Library<"memapi"> {
    public static function create_file_mapping(filename:String, mapname:String, sizeHi:Int, sizeLo:Int):LibFileMapping;
    public static function open_file_mapping(mapname:String):LibFileMapping;
    public static function remap_view(map:LibFileMapping, view:LibMapView):Bool;
    public static function mapview_stub():LibMapView;
    public static function write_to_view(view:LibMapView, offset_hi:UInt, offset_lo:UInt, data:haxe.io.Bytes, len:SizeOf<"data">):Bool;
    public static function read_view(view:LibMapView, offset_hi:UInt, offset_lo:UInt, dLen:SizeOfReturn):haxe.io.Bytes;
    public static function get_sys_granularity():UInt;
}
@:ammer.nativePrefix("mem_")
class LibFileMapping extends Pointer<"mem_mapping_t", MemLib> {
    public function create_view(_:This, offsetHi:Int, offsetLo:Int, size:Int):LibMapView;
    public function mapping_errored(_:This):Bool;
    public function mapping_get_errors(_:This):String;
}
@:ammer.nativePrefix("mem_")
class LibMapView extends Pointer<"mem_mapview_t", MemLib> {
    
    public function mapview_errored(_:This):Bool;
    public function mapview_get_errors(_:This):String;
    public function persist_view(_:This):Bool;
}
