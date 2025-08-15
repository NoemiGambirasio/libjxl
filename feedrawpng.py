import PIL
from PIL import Image
import subprocess
import jax.numpy as jnp
import jax

def feed_raw_png(input_path, output_path):
    """
    Feed a raw PNG file to the encoder.
    
    Args:
        input_path (str): Path to the input PNG file.
        output_path (str): Path to save the output encoded file.
    """
    # Open the image using PIL
    with Image.open(input_path) as img:
        # Convert image to raw format
        arr = jax.Array.flatten(jnp.asarray(img, dtype=jnp.float32))
        with open(output_path, 'w') as f:
            f.write(f"{img.width}\n")
            f.write(f"{img.height}\n")
            for i in arr:
                f.write(f"{i}\n")
            f.close()


high_res = input("high_res:")
low_res = input("low_res")

feed_raw_png(high_res,high_res.replace(".png",".txt"))
feed_raw_png(low_res,low_res.replace(".png",".txt"))

subprocess.run(['./build/lib/jxl_encoder',high_res.replace(".png",".txt"),low_res.replace(".png",".txt")], check=True)