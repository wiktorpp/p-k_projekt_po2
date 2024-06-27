/**
 * @file main.cpp
 * @brief RLE Encoder/Decoder GUI Application using GTK.
 * 
 * This file contains the implementation of a simple GTK-based GUI application 
 * for Run-Length Encoding (RLE) and decoding of text and files.
 */

#include <gtk/gtk.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>

GtkBuilder *builder;          ///< GTK builder for loading the GUI from the Glade file.
GtkWidget *main_window;       ///< Main application window.
GtkWidget *about_window;      ///< About dialog window.
GtkWidget *text_entry;        ///< Text entry widget for input/output text.

/**
 * @brief Convert a vector of bytes to a hex string.
 * 
 * @param bytes Vector of bytes to convert.
 * @return Hex string representation of the bytes.
 */
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<unsigned>(byte);
    }
    
    return ss.str();
}

/**
 * @brief Convert a hex string to a vector of bytes.
 * 
 * @param hex Hex string to convert.
 * @return Vector of bytes represented by the hex string.
 */
std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        unsigned byte;
        std::istringstream(hex.substr(i, 2)) >> std::hex >> byte;
        bytes.push_back(static_cast<unsigned char>(byte));
    }
    
    return bytes;
}

/**
 * @brief Encode data using Run-Length Encoding (RLE).
 * 
 * @param data Vector of bytes to encode.
 * @return RLE encoded vector of bytes.
 */
std::vector<unsigned char> encode_rle(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> encoded;
    size_t count = 1;
    
    for (size_t i = 1; i <= data.size(); ++i) {
        if (i == data.size() || data[i] != data[i - 1]) {
            encoded.push_back(count);
            encoded.push_back(data[i - 1]);
            count = 1;
        } else {
            ++count;
        }
    }
    
    return encoded;
}

/**
 * @brief Decode data from Run-Length Encoding (RLE).
 * 
 * @param encoded RLE encoded vector of bytes.
 * @return Decoded vector of bytes.
 */
std::vector<unsigned char> decode_rle(const std::vector<unsigned char>& encoded) {
    std::vector<unsigned char> decoded;
    
    for (size_t i = 0; i < encoded.size(); i += 2) {
        unsigned char count = encoded[i];
        unsigned char byte = encoded[i + 1];
        
        for (int j = 0; j < count; ++j) {
            decoded.push_back(byte);
        }
    }
    
    return decoded;
}

/**
 * @brief Encode input text using RLE and convert to hex string.
 * 
 * @param input Input text string.
 * @return Hex string of the RLE encoded input text.
 */
std::string encode_rle_hex(const std::string& input) {
    std::vector<unsigned char> data(input.begin(), input.end());
    std::vector<unsigned char> encoded = encode_rle(data);
    return bytes_to_hex(encoded);
}

/**
 * @brief Decode hex string from RLE encoding.
 * 
 * @param hex Hex string of RLE encoded data.
 * @return Decoded text string.
 */
std::string decode_rle_hex(const std::string& hex) {
    std::vector<unsigned char> encoded = hex_to_bytes(hex);
    std::vector<unsigned char> decoded = decode_rle(encoded);
    return std::string(decoded.begin(), decoded.end());
}

/**
 * @brief Callback function for the About button click event.
 * 
 * @param button GTK button that was clicked.
 * @param user_data User data passed to the callback function.
 */
void on_about_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *about_window = GTK_WIDGET(user_data);
    gtk_widget_show_all(about_window);
}

/**
 * @brief Perform text encoding or decoding action based on action type.
 * 
 * @param action_type Action type: 0 for encoding, 1 for decoding.
 * @param text_entry_widget GTK text entry widget containing the text.
 */
void text_action(int action_type, GtkWidget *text_entry_widget) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(text_entry_widget));
    std::string input_text = text;
    std::string result_text;

    if (action_type == 0) {
        result_text = encode_rle_hex(input_text);
    } else if (action_type == 1) {
        result_text = decode_rle_hex(input_text);
    } else {
        g_printerr("Invalid action type.\n");
        return;
    }

    gtk_entry_set_text(GTK_ENTRY(text_entry_widget), result_text.c_str());
}

/**
 * @brief Perform file encoding or decoding action based on action type.
 * 
 * @param action_type Action type: 0 for encoding, 1 for decoding.
 * @param parent_window Parent GTK window.
 * @param action GTK file chooser action.
 */
void file_action(int action_type, GtkWidget *parent_window, GtkFileChooserAction action) {
    GtkWidget *dialog;
    gint res;

    dialog = gtk_file_chooser_dialog_new(
        (action_type == 0) ? "Encode file" : "Save File",
        GTK_WINDOW(parent_window),
        action,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        (action_type == 0) ? "_Encode" : "_Decode",
        GTK_RESPONSE_ACCEPT,
        NULL
    );

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        std::ifstream file_in(filename, std::ios::binary | std::ios::ate);
        std::ofstream file_out;
        std::vector<unsigned char> data;
        std::string output_filename;

        if (file_in.is_open()) {
            std::streamsize size = file_in.tellg();
            file_in.seekg(0, std::ios::beg);
            data.resize(size);
            file_in.read(reinterpret_cast<char*>(&data[0]), size);

            if (action_type == 0) {
                std::vector<unsigned char> encoded = encode_rle(data);
                output_filename = std::string(filename) + ".encoded";
                file_out.open(output_filename, std::ios::binary);
                file_out.write(reinterpret_cast<const char*>(&encoded[0]), encoded.size());
            } else {
                std::vector<unsigned char> decoded = decode_rle(data);
                output_filename = std::string(filename) + ".decoded";
                file_out.open(output_filename, std::ios::binary);
                file_out.write(reinterpret_cast<const char*>(&decoded[0]), decoded.size());
            }

            file_out.close();
        } else {
            g_printerr("Failed to open file.\n");
        }

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

/**
 * @brief Callback function for the Encode button click event.
 * 
 * @param button GTK button that was clicked.
 * @param user_data User data passed to the callback function.
 */
void on_encode_clicked(GtkButton *button, gpointer user_data) {
    text_action(0, GTK_WIDGET(user_data));
}

/**
 * @brief Callback function for the Decode button click event.
 * 
 * @param button GTK button that was clicked.
 * @param user_data User data passed to the callback function.
 */
void on_decode_clicked(GtkButton *button, gpointer user_data) {
    text_action(1, GTK_WIDGET(user_data));
}

/**
 * @brief Callback function for the Encode File button click event.
 * 
 * @param button GTK button that was clicked.
 * @param user_data User data passed to the callback function.
 */
void on_encode_file_clicked(GtkButton *button, gpointer user_data) {
    file_action(0, GTK_WIDGET(user_data), GTK_FILE_CHOOSER_ACTION_SAVE);
}

/**
 * @brief Callback function for the Decode File button click event.
 * 
 * @param button GTK button that was clicked.
 * @param user_data User data passed to the callback function.
 */
void on_decode_file_clicked(GtkButton *button, gpointer user_data) {
    file_action(1, GTK_WIDGET(user_data), GTK_FILE_CHOOSER_ACTION_SAVE);
}

/**
 * @brief Main function to initialize the GTK application and run the main loop.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit status code.
 */
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("gui.glade");
    if (builder == NULL) {
        g_printerr("Error loading builder file.\n");
        return 1;
    }

    main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    about_window = GTK_WIDGET(gtk_builder_get_object(builder, "about_window"));
    text_entry = GTK_WIDGET(gtk_builder_get_object(builder, "text_entry"));

    if (main_window == NULL || about_window == NULL || text_entry == NULL) {
        g_printerr("Error retrieving GTK widgets.\n");
        return 1;
    }

    gtk_builder_connect_signals(builder, text_entry);
    
    g_signal_connect(gtk_builder_get_object(builder, "about_button"), "clicked", G_CALLBACK(on_about_clicked), about_window);
    g_signal_connect(gtk_builder_get_object(builder, "encode_button"), "clicked", G_CALLBACK(on_encode_clicked), text_entry);
    g_signal_connect(gtk_builder_get_object(builder, "decode_button"), "clicked", G_CALLBACK(on_decode_clicked), text_entry);
    g_signal_connect(gtk_builder_get_object(builder, "encode_file_button"), "clicked", G_CALLBACK(on_encode_file_clicked), about_window);
    g_signal_connect(gtk_builder_get_object(builder, "decode_file_button"), "clicked", G_CALLBACK(on_decode_file_clicked), about_window);

    gtk_widget_show_all(main_window);
    gtk_main();
    return 0;
}
