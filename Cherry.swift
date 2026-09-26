//
//  Cherry.swift
//  Cherry
//
//  Created by Jarrod Norwell on 2/9/2026.
//

import Foundation

public enum CherryButton : UInt32, Codable {
    case up = 0x001,
         down = 0x002,
         left = 0x004,
         right = 0x008,
         l = 0x010,
         r = 0x020,
         num1 = 0x040,
         num2 = 0x080,
         num3 = 0x100,
         num4 = 0x200,
         num5 = 0x400,
         num6 = 0x800,
         num7 = 0x1000,
         num8 = 0x2000,
         num9 = 0x4000,
         num0 = 0x8000,
         star = 0x10000,
         pound = 0x20000
    
    var uint32: UInt32 { rawValue }
}

public class CherryCommon {
    public init() {}
    
    public static var documentDirectoryURL: URL? {
        FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first
    }
    
    public static var cherryDirectoryURL: String? {
        if let documentDirectoryURL {
            documentDirectoryURL.appending(component: "Cherry").path
        } else {
            nil
        }
    }
}

public actor CherrySystem {
    private var fileManager: FileManager = .default
    
    public init() {}
    
    public func printAbout() {
        cherry.print_about()
    }
    
    public func initializePaths() {
        cherry.initialize_paths()
    }
    
    public func initializeSystem() {
        cherry.initialize_system()
    }
    
    public func destroySystem() {
        cherry.destroy_system()
    }
    
    public func insertDisc(at url: URL) {
        cherry.insert_disc(std.string(url.path))
    }
    
    public func set(change: Bool = false, isRunning: Bool = false) {
        if change {
            running = isRunning
        }
    }
    
    public var running: Bool {
        get {
            cherry.is_running()
        }
        set {
            cherry.is_running(true, newValue)
        }
    }
    
    public func set(change: Bool = false, isPaused: Bool = false) {
        if change {
            paused = isPaused
        }
    }
    
    public var paused: Bool {
        get {
            cherry.is_paused()
        }
        set {
            cherry.is_paused(true, newValue)
        }
    }
    
    
    public func start() {
        cherry.start()
    }
    
    public func stop() {
        cherry.stop()
    }
    
    
    public var framebufferHeight: Int32 {
        cherry.framebuffer_height()
    }
    
    public var framebufferWidth: Int32 {
        cherry.framebuffer_width()
    }
    
    
    public nonisolated func press(button: CherryButton, index: Int32 = 0) {
        cherry.press_button(button.uint32)
    }
    
    public nonisolated func release(button: CherryButton, index: Int32 = 0) {
        cherry.release_button(button.uint32)
    }
    
    
    public nonisolated func audioBuffer(callback: cherry.AudioVideoBufferCallback) {
        cherry.audio_buffer_callback(callback)
    }
    
    public nonisolated func videoBuffer(callback: cherry.AudioVideoBufferCallback) {
        cherry.video_buffer_callback(callback)
    }
    
    
    public func setContext(context: UnsafeMutableRawPointer) {
        cherry.set_context(context)
    }
    
    
    public nonisolated func boxartURLString(for url: URL) -> String? {
        var title: String = url.deletingPathExtension().lastPathComponent
        title = title.replacingOccurrences(of: "&", with: "_")
        
        let repository: String = "https://raw.githubusercontent.com/libretro/libretro-thumbnails"
        let path: String = "Coleco - ColecoVision/Named_Boxarts"
        
        return "\(repository)/refs/heads/master/\(path)/\(title).png"
    }
}
