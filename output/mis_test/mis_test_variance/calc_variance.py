import numpy as np
from PIL import Image

# Load an image and convert it to a NumPy array
def load_image_as_array(filepath):
    img = Image.open(filepath).convert('L')  # Convert image to grayscale
    return np.array(img, dtype=np.float32)  # Convert to float for calculations

# Compute variance of pixel differences between two images
def compute_variance(image1, image2):
    difference = image1 - image2  # Pixel-wise difference
    return np.var(difference)  # Compute variance of the differences

# Paths to the BMP images
reference_path = "mis_test_8192spp_334s_naive.bmp"
nee_path = "mis_test_16spp_3s_nee.bmp"
mis_path = "mis_test_16spp_4s_mis.bmp"

# Load the images
reference = load_image_as_array(reference_path)
nee = load_image_as_array(nee_path)
mis = load_image_as_array(mis_path)

# Compute variances
variance_nee = compute_variance(reference, nee)
variance_mis = compute_variance(reference, mis)

print(f"NEE variance: {variance_nee}")
print(f"MIS variance: {variance_mis}")
