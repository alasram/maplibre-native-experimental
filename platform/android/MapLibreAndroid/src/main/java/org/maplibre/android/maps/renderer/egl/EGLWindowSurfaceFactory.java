package org.maplibre.android.maps.renderer.egl;

import android.opengl.GLSurfaceView;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import java.io.File;
import java.lang.ProcessBuilder;

public class EGLWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory {


  public static class MemInfoLoggerJ {

    private String msg;

    private void runCommand(String cmd) {
      try {
      Runtime runtime = Runtime.getRuntime();
      runtime.exec(cmd);
      Log.e("EGLWindowSurfaceFactory", cmd);
      } catch (Exception exception) {
        Log.e("EGLWindowSurfaceFactory", "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ failed TO EXECUTE: " + cmd, exception);
      }
    }

    private synchronized void print(Boolean begin) {

        String text = "######################################################## ";
        text += begin ? "BEGIN " : "END ";
        text += msg + " ";

        String cmd = "echo \"" + text + "\" >> /data/testdir/mem_info.txt";
        runCommand(cmd);
        runCommand("dumpsys meminfo com.rivian.rivianivinavigation >> /data/testdir/mem_info.txt");
    }

    public MemInfoLoggerJ(String s){
        msg = s;
        print(true);
    }

    public void end() {
        print(false);
    }

  };


  public EGLSurface createWindowSurface(@NonNull EGL10 egl, @Nullable EGLDisplay display, @Nullable EGLConfig config,
                                        @Nullable Object nativeWindow) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("EGLWindowSurfaceFactory::createWindowSurface");

    EGLSurface result = null;
    if (display != null && config != null && nativeWindow != null) {
      try {
        result = egl.eglCreateWindowSurface(display, config, nativeWindow, null);
      } catch (Exception exception) {
        // This exception indicates that the surface flinger surface
        // is not valid. This can happen if the surface flinger surface has
        // been torn down, but the application has not yet been
        // notified via SurfaceHolder.Callback.surfaceDestroyed.
        // In theory the application should be notified first,
        // but in practice sometimes it is not. See b/4588890
        Log.e("EGLWindowSurfaceFactory", "eglCreateWindowSurface", exception);
      }
    }

    logger.end();
    return result;
  }

  public void destroySurface(EGL10 egl, EGLDisplay display,
                             EGLSurface surface) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("EGLWindowSurfaceFactory::destroySurface");

    egl.eglDestroySurface(display, surface);

    logger.end();
  }
}
