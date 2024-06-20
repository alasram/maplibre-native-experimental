package org.maplibre.android.maps.renderer;

import android.content.Context;

import androidx.annotation.CallSuper;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import org.maplibre.android.LibraryLoader;
import org.maplibre.android.log.Logger;
import org.maplibre.android.maps.MapLibreMap;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.util.Log;
import java.io.File;
import java.lang.ProcessBuilder;

/**
 * The {@link MapRenderer} encapsulates the GL thread.
 * <p>
 * Performs actions on the GL thread to manage the GL resources and
 * render on the one end and acts as a scheduler to request work to
 * be performed on the GL thread on the other.
 */
@Keep
public abstract class MapRenderer implements MapRendererScheduler {

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

  static {
    LibraryLoader.load();
  }

  private static final String TAG = "Mbgl-MapRenderer";

  // Holds the pointer to the native peer after initialisation
  private long nativePtr = 0;
  private double expectedRenderTime = 0;
  private MapLibreMap.OnFpsChangedListener onFpsChangedListener;

  public MapRenderer(@NonNull Context context, String localIdeographFontFamily) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::MapRenderer");

    float pixelRatio = context.getResources().getDisplayMetrics().density;

    // Initialise native peer
    nativeInitialize(this, pixelRatio, localIdeographFontFamily);
  }

  public void onStart() {
    // Implement if needed
  }

  public void onPause() {
    // Implement if needed
  }

  public void onResume() {
    // Implement if needed
  }

  public void onStop() {
    // Implement if needed
  }

  public void onDestroy() {
    // Implement if needed
  }

  public void setOnFpsChangedListener(MapLibreMap.OnFpsChangedListener listener) {
    onFpsChangedListener = listener;
  }

  @CallSuper
  protected void onSurfaceCreated(GL10 gl, EGLConfig config) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::onSurfaceCreated");
    
    nativeOnSurfaceCreated();
  }

  @CallSuper
  protected void onSurfaceChanged(@NonNull GL10 gl, int width, int height) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::onSurfaceChanged");

    gl.glViewport(0, 0, width, height);
    nativeOnSurfaceChanged(width, height);
  }

  @CallSuper
  protected void onSurfaceDestroyed() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::onSurfaceDestroyed");
    nativeOnSurfaceDestroyed();
  }

  @CallSuper
  protected void onDrawFrame(GL10 gl) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::onDrawFrame");

    long startTime = System.nanoTime();
    try {
      nativeRender();
    } catch (java.lang.Error error) {
      Logger.e(TAG, error.getMessage());
    }
    long renderTime = System.nanoTime() - startTime;
    if (renderTime < expectedRenderTime) {
      try {
        Thread.sleep((long) ((expectedRenderTime - renderTime) / 1E6));
      } catch (InterruptedException ex) {
        Logger.e(TAG, ex.getMessage());
      }
    }
    if (onFpsChangedListener != null) {
      updateFps();
    }
  }

  public void setSwapBehaviorFlush(boolean flush) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::setSwapBehaviorFlush");

    nativeSetSwapBehaviorFlush(flush);
  }

  /**
   * May be called from any thread.
   * <p>
   * Called from the native peer to schedule work on the GL
   * thread. Explicit override for easier to read jni code.
   *
   * @param runnable the runnable to execute
   * @see MapRendererRunnable
   */
  @CallSuper
  void queueEvent(MapRendererRunnable runnable) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::queueEvent");

    this.queueEvent((Runnable) runnable);
  }

  /// Wait indefinitely for the queue to become empty
  public void waitForEmpty() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("MapRenderer::waitForEmpty");

    waitForEmpty(0);
  }

  private native void nativeInitialize(MapRenderer self,
                                       float pixelRatio,
                                       String localIdeographFontFamily);

  @CallSuper
  @Override
  protected native void finalize() throws Throwable;

  private native void nativeOnSurfaceCreated();

  private native void nativeOnSurfaceChanged(int width, int height);

  private native void nativeOnSurfaceDestroyed();

  protected native void nativeReset();

  private native void nativeRender();

  private native void nativeSetSwapBehaviorFlush(boolean flush);

  private long timeElapsed;

  private void updateFps() {
    long currentTime = System.nanoTime();
    if (timeElapsed > 0) {
      double fps = 1E9 / ((currentTime - timeElapsed));
      onFpsChangedListener.onFpsChanged(fps);
    }
    timeElapsed = currentTime;
  }

  /**
   * The max frame rate at which this render is rendered,
   * but it can't excess the ability of device hardware.
   *
   * @param maximumFps Can be set to arbitrary integer values.
   */
  public void setMaximumFps(int maximumFps) {
    if (maximumFps <= 0) {
      // Not valid, just return
      return;
    }
    expectedRenderTime = 1E9 / maximumFps;
  }
}
