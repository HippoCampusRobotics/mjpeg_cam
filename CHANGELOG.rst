^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package mjpeg_cam
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
-----------
* moved to separate repo
* streamlined ov9281 launch
* ditched yapf
* Better exception info text
* moved code to visual_localization
* make image_proc container name unique for each camera
  hopefully avoids name clashes
* setting camera frame_id via namespace
* fixed wrong camera frame. use calibration file for frame id.
* finished rectification pipeline
* added image_decoder
* changed qos to best effort to improve performance
* fixed compilation error
* set framerate during device initialization
  while capturing the framerate cannot be set anymore
* apply frame rate settings
* mjpeg_cam update
* added pre-commit hooks
* added licenses and applied formatting to all source files
* added some parameter magic and fixed wrong message size
* added initial verison of mjpeg_cam
* Contributors: Thies Lennart Alff
