#define LOGARITHMIC_GROWTH

#include "tizensensorreader.h"
#include <sensor.h>
#include <vector.h>

#define ACCELEROMETER_INTERVAL_MS 20

typedef struct sensor_event_s
{
    int accuracy;
    unsigned long long timestamp;
    int value_count;
    float values[MAX_VALUE_SIZE];
};

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *button;
	Evas_Object *box;
	Evas_Object *nf;
	sensor_listener_h accelerationListener;
	bool flag;
	sensor_event_s *accData;
} appdata_s;

static void
accelerometer_cb(sensor_h sensor, sensor_event_s *event, void *data){

    appdata_s * ad = (appdata_s *)data;

    /*  Some game calculations like reflections from the edge,
	calculation of current speed and resistance of motion.
        Acceleration for each axis:
        	(float)event->values[0]
        	(float)event->values[1]
        	(float)event->values[2]
    */
    printf("%f",event->values[0]);
    vector_push_back(ad->accData, *event);
    dlog_print(DLOG_INFO, "USR_TAG", "%d", event->timestamp);
}

static int
un_register_accelerometer_callback(appdata_s *ad){
	int error;
	error = sensor_listener_stop(ad->accelerationListener);
	error = sensor_destroy_listener(ad->accelerationListener);
	return error;
}


static int
register_accelerometer_callback(appdata_s *ad)
{
    int error;
    bool supported;
    sensor_h accelerometer;

    error = sensor_is_supported( SENSOR_ACCELEROMETER, &supported );
    if(error != SENSOR_ERROR_NONE && supported){
     return error;
    }

    error = sensor_get_default_sensor(SENSOR_ACCELEROMETER, &accelerometer);
    if(error != SENSOR_ERROR_NONE){
     return error;
    }

    error = sensor_create_listener( accelerometer, &ad->accelerationListener);
    if(error != SENSOR_ERROR_NONE){
     return error;
    }

    error = sensor_listener_set_event_cb( ad->accelerationListener,
            ACCELEROMETER_INTERVAL_MS, accelerometer_cb, ad );
    if(error != SENSOR_ERROR_NONE){
     return error;
    }

    sensor_listener_set_attribute_int(ad->accelerationListener, SENSOR_ATTRIBUTE_PAUSE_POLICY, SENSOR_PAUSE_NONE);
    error = sensor_listener_set_option(ad->accelerationListener, SENSOR_OPTION_ALWAYS_ON);

    error = sensor_listener_start( ad->accelerationListener );

    return SENSOR_ERROR_NONE;
}

void
clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s * ad = (appdata_s *)data;
	if(ad->flag){
		un_register_accelerometer_callback(ad);
		ad->flag = false;
		elm_object_text_set(ad->button, "Start Record");
		dlog_print(DLOG_INFO, "USR_TAG", "Unregistered\n");
		dlog_print(DLOG_INFO, "USR_TAG", "size: %d\n", vector_size(ad->accData));
	}else{
		register_accelerometer_callback(ad);
		ad->flag = true;
		elm_object_text_set(ad->button, "Stop Record");
		dlog_print(DLOG_INFO, "USR_TAG", "Registered\n");
	}

}

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{

	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	//if (elm_win_wm_rotation_supported_get(ad->win)) {
	//	int rots[4] = { 0, 90, 180, 270 };
	//	elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	//}

	//evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	//eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_conformant_set(ad->win, EINA_TRUE);
	//elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	//elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	// Naviframe
	ad->nf = elm_naviframe_add(ad->conform);
	elm_object_content_set(ad->conform, ad->nf);
	evas_object_show(ad->nf);

	// Add the box
	ad->box = elm_box_add(ad->nf);
	evas_object_size_hint_weight_set(ad->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* Label */
	/* Create an actual view of the base gui.
	   Modify this part to change the view. */
	//ad->label = elm_label_add(ad->conform);
	ad->label = elm_label_add(ad->box);
	elm_object_text_set(ad->label, "<align=center>Data size: 0</align>");
	//evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_object_content_set(ad->conform, ad->label);
	elm_box_pack_end(ad->box, ad->label);
	evas_object_show(ad->label);

	/* Show window after base gui is set up */

	//ad->button = elm_button_add(ad->box);
	ad->button = elm_button_add(ad->box);
	elm_object_style_set(ad->button, "default");
	elm_object_text_set(ad->button, "Start Record");
	evas_object_smart_callback_add(ad->button, "clicked", clicked_cb, ad);
	evas_object_size_hint_weight_set(ad->button, EVAS_HINT_EXPAND, 0.1);
	evas_object_size_hint_align_set(ad->button, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(ad->box, ad->button);
	evas_object_show(ad->button);

	elm_naviframe_item_push(ad->nf, _("Sensor Reader"), NULL, NULL, ad->box, "basic");

	evas_object_show(ad->box);
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	ad->flag = false;
	ad->accData = NULL;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}



	return ret;
}
