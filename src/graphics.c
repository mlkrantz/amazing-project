#include <gtk/gtk.h>

int main(int argc, char* argv[])
{
  gtk_init_check(&argc, &argv);

  //initialize window and socket
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GtkWidget *socket = gtk_socket_new();

  //attach window to socket
  gtk_container_add(GTK_CONTAINER(window), socket);

  //set size fo
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
  gtk_widget_show_all(window);

  g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),window);
  GdkNativeWindow windowID = gtk_socket_get_id(GTK_SOCKET(socket));
  long unsigned int test=windowID;
  g_print("%lu\n", test);

  gtk_main();
  return 0;
}
