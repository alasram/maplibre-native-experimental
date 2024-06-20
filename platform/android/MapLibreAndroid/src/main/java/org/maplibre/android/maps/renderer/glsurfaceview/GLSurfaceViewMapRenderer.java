package org.maplibre.android.maps.renderer.glsurfaceview;

import android.content.Context;
import android.opengl.GLSurfaceView;

import androidx.annotation.NonNull;

import org.maplibre.android.maps.renderer.MapRenderer;
import org.maplibre.android.maps.renderer.egl.EGLConfigChooser;
import org.maplibre.android.maps.renderer.egl.EGLContextFactory;
import org.maplibre.android.maps.renderer.egl.EGLWindowSurfaceFactory;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLSurfaceView.RENDERMODE_WHEN_DIRTY;

import android.util.Log;
import java.io.File;
import java.lang.ProcessBuilder;


/**
 * The {@link GLSurfaceViewMapRenderer} encapsulates the GL thread and
 * {@link GLSurfaceView} specifics to render the map.
 *
 * @see MapRenderer
 */
public class GLSurfaceViewMapRenderer extends MapRenderer implements GLSurfaceView.Renderer {

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


  @NonNull
  private final MapLibreGLSurfaceView glSurfaceView;

  public GLSurfaceViewMapRenderer(Context context,
                                  MapLibreGLSurfaceView glSurfaceView,
                                  String localIdeographFontFamily) {
    super(context, localIdeographFontFamily);

    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::GLSurfaceViewMapRenderer");

    this.glSurfaceView = glSurfaceView;
    glSurfaceView.setEGLContextFactory(new EGLContextFactory());
    glSurfaceView.setEGLWindowSurfaceFactory(new EGLWindowSurfaceFactory());
    glSurfaceView.setEGLConfigChooser(new EGLConfigChooser());
    glSurfaceView.setRenderer(this);
    glSurfaceView.setRenderMode(RENDERMODE_WHEN_DIRTY);
    glSurfaceView.setPreserveEGLContextOnPause(true);
    glSurfaceView.setDetachedListener(new MapLibreGLSurfaceView.OnGLSurfaceViewDetachedListener() {
      @Override
      public void onGLSurfaceViewDetached() {
        // because the GL thread is destroyed when the view is detached from window,
        // we need to ensure releasing the native renderer as well.
        // This avoids releasing it only when the view is being recreated, which is already on a new GL thread,
        // and leads to JNI crashes like https://github.com/mapbox/mapbox-gl-native/issues/14618
        nativeReset();
      }
    });

    logger.end();
  }

  @Override
  public void onStop() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onStop");
    glSurfaceView.onPause();
    logger.end();
  }

  @Override
  public void onPause() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onPause");
    super.onPause();
    logger.end();
  }

  @Override
  public void onDestroy() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onDestroy");
    super.onDestroy();
    logger.end();
  }

  @Override
  public void onStart() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onStart");
    glSurfaceView.onResume();
    logger.end();
  }

  @Override
  public void onResume() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onResume");
    super.onResume();
    logger.end();
  }

  @Override
  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onSurfaceCreated");
    super.onSurfaceCreated(gl, config);
    logger.end();
  }

  @Override
  protected void onSurfaceDestroyed() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onSurfaceDestroyed");
    super.onSurfaceDestroyed();
    logger.end();
  }

  @Override
  public void onSurfaceChanged(GL10 gl, int width, int height) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onSurfaceChanged");
    super.onSurfaceChanged(gl, width, height);
    logger.end();
  }

  @Override
  public void onDrawFrame(GL10 gl) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::onDrawFrame");
    super.onDrawFrame(gl);
    logger.end();
  }

  /**
   * May be called from any thread.
   * <p>
   * Called from the renderer frontend to schedule a render.
   */
  @Override
  public void requestRender() {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::requestRender");
    glSurfaceView.requestRender();
    logger.end();
  }

  /**
   * May be called from any thread.
   * <p>
   * Schedules work to be performed on the MapRenderer thread.
   *
   * @param runnable the runnable to execute
   */
  @Override
  public void queueEvent(Runnable runnable) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::queueEvent");
    glSurfaceView.queueEvent(runnable);
    logger.end();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public long waitForEmpty(long timeoutMillis) {
    MemInfoLoggerJ logger = new MemInfoLoggerJ("GLSurfaceViewMapRenderer::waitForEmpty");
    return glSurfaceView.waitForEmpty(timeoutMillis);
  }
}