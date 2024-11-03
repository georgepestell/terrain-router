import cv2
import numpy as np
from matplotlib import pyplot as plt

# Load the image
image = cv2.imread('benNevis_terrain_types_UTM.tiff')

# Initialize an empty image for storing the final contours
hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
blurred_image = cv2.GaussianBlur(hsv_image, (13, 13), 0)
contour_image = np.copy(blurred_image)

# Find edges using Canny
edges = cv2.Canny(blurred_image, 1, 20)

# Find contours from the edges
contours, _ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

# Step 5: Simplify contours
simplified_contours = []
epsilon_factor = 0.05  # Adjust this factor for more or less simplification

for contour in contours:
    epsilon = epsilon_factor * cv2.arcLength(contour, True)
    simplified_contour = cv2.approxPolyDP(contour, epsilon, True)
    simplified_contours.append(simplified_contour)

# Draw contours on the original image (with different colors for each region)
cv2.drawContours(contour_image, contours, -1, (0, 0, 0), 1)  # Green contours for example

print(len(contours))

# Display the result
plt.imshow(cv2.cvtColor(contour_image, cv2.COLOR_BGR2RGB))
plt.title('Contours by Color')
plt.show()
plt.imshow(cv2.cvtColor(blurred_image, cv2.COLOR_HSV2RGB))
plt.title('Contours by Color')
plt.show()
cv2.waitKey(0)
cv2.destroyAllWindows()
