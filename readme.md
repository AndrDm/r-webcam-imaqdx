## Simple Web Cam Grab:

Minimal example of image acquisition using **NI IMAQdx** and **NI Vision**
via Rust FFI bindings.

### Prerequisites

- NI Vision Development Module installed
- NI IMAQdx installed
- Camera configured and visible in NI MAX
- Camera name must match NI MAX (default: `cam1`

```rust
#![allow(unused_imports)]
mod nivision;
use crate::nivision::*;
mod NIIMAQdx;
use crate::NIIMAQdx::*;

use std::{ffi::CString, ptr::null_mut};

static mut IMAGE_SRC: *mut nivision::Image = null_mut();

fn main() {
	println!("Hello, webcam!");
	unsafe {
		imaqSetWindowThreadPolicy(WindowThreadPolicy_enum_IMAQ_SEPARATE_THREAD);
		IMAGE_SRC = imaqCreateImage(ImageType_enum_IMAQ_IMAGE_RGB, 0);
		imaqSetWindowZoomToFit(0, 1);

		// Open camera; cam1 by default
		let camera_name = CString::new("cam1").unwrap();
		let mut session: IMAQdxSession = 0;

		IMAQdxOpenCamera(
			camera_name.as_ptr(),
			IMAQdxCameraControlMode_enum_IMAQdxCameraControlModeController,
			&mut session,
		);

		IMAQdxConfigureGrab(session);
		let mut buffer_number: NIIMAQdx::uInt32 = 0;
		loop {
			IMAQdxGrab(
				session,
				IMAGE_SRC as *mut NIIMAQdx::Image,
				1, //wait for next buffer
				&mut buffer_number,
			);
			println!("Buffer (Frame) number: {}", buffer_number);

			imaqDisplayImage(IMAGE_SRC, 0, 0);
		}
	}
}
// IMAQdxCloseCamera (session);
```

Error handling and cleanup (IMAQdxCloseCamera, imaqDispose) are omitted for clarity, this is just simple basic demo.
