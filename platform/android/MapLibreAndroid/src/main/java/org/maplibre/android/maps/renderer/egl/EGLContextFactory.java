package org.maplibre.android.maps.renderer.egl;

import android.opengl.GLSurfaceView;
import android.util.Log;

import androidx.annotation.Nullable;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import java.io.File;
import java.lang.ProcessBuilder;

public class EGLContextFactory implements GLSurfaceView.EGLContextFactory {

public static class MemInfoLoggerJ {

    private String msg;

    private void runCommand(String cmd) {
      try {
      Runtime runtime = Runtime.getRuntime();
      runtime.exec(cmd);
      Log.e("NS_DBG", cmd);
      } catch (Exception exception) {
        // Log.e(TAG, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ failed TO EXECUTE: " + cmd, exception);
      }
    }

    private void runEcho(String cmd) {
      try {
        File out_file = new File("/data/testdir/mem_info.txt");
        ProcessBuilder builder = new ProcessBuilder("/system/bin/echo", cmd);
        builder.redirectOutput(ProcessBuilder.Redirect.appendTo(out_file));
        builder.redirectError(ProcessBuilder.Redirect.appendTo(out_file));
        Process p = builder.start(); // may throw IOException
        Log.e("NS_DBG", cmd);
        p.waitFor();
      } catch (Exception exception) {
        Log.e("NS_DBG", "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ failed TO EXECUTE echo: " + cmd, exception);
      }
    }

    private void runDumpsys() {
      try {
        File out_file = new File("/data/testdir/mem_info.txt");
        ProcessBuilder builder = new ProcessBuilder("/system/bin/dumpsys", "meminfo", "com.rivian.rivianivinavigation");
        builder.redirectOutput(ProcessBuilder.Redirect.appendTo(out_file));
        builder.redirectError(ProcessBuilder.Redirect.appendTo(out_file));
        Process p = builder.start(); // may throw IOException
        // Log.e("DefaultContextFactory", cmd);
        p.waitFor();
      } catch (Exception exception) {
        Log.e("NS_DBG", "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ failed TO EXECUTE dumpsys: " + msg, exception);
      }
    }

    private synchronized void print(Boolean begin) {

        String text = "######################################################## ";
        text += begin ? "BEGIN " : "END ";
        text += msg + " ";

        runEcho(text);
        runDumpsys();

        // String cmd = "echo \"" + text + "\" >> /data/testdir/mem_info.txt";
        // runCommand(cmd);
        // runCommand("dumpsys meminfo com.rivian.rivianivinavigation >> /data/testdir/mem_info.txt");
    }

    public MemInfoLoggerJ(String s){
        msg = s;
        print(true);
    }

    public void end() {
        print(false);
    }

  };

  public EGLContext createContext(EGL10 egl, @Nullable EGLDisplay display, @Nullable EGLConfig config) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("EGLContextFactory::createContext");

    if (display == null || config == null) {
      return EGL10.EGL_NO_CONTEXT;
    }
    int[] attrib_list = {0x3098, 2, EGL10.EGL_NONE};
    return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attrib_list);
  }

  public void destroyContext(EGL10 egl, EGLDisplay display,
                             EGLContext context) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("EGLContextFactory::destroyContext");

    if (!egl.eglDestroyContext(display, context)) {
      Log.e("DefaultContextFactory", "display:" + display + " context: " + context);
      Log.i("DefaultContextFactory", "tid=" + Thread.currentThread().getId());
    }
  }
}
