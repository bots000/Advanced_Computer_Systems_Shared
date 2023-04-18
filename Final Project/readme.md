Work Completed:
- simple greyscale conversion
- simd greyscale conversion (about 33% improvement, credit to this dude: https://stackoverflow.com/questions/57832444/efficient-c-code-no-libs-for-image-transformation-into-custom-rgb-pixel-grey)
- simple greyscale threshold determination
- simd greyscale threshold determination compatability

Comparing running simple vs simd for all 300 ish test images at one time (simd only for converting to greyscale, time only recorded for the actual conversion):
- Simd took 13.687 seconds
- RGB version took 15.099 seconds
- Simple took 16.586 seconds

Work to be completed:

- simple RGB threshold determination
- simd RGB threshold determination compatability
