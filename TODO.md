## shexp todo

- [x] framework
- [x] sphere set approximation
- [x] sphere hierachy
- [x] avoid self-shadowing
- [ ] receiver point clustering
- [ ] blocker selection & ratio vector
- [X] L*H(N) cubemap (w/ windowing)
- [x] shlog table
- [x] sh rotation
- [x] shexp



per-vertex shading

```
[on CPU]
node: spheres, {child nodes}

// form hierarchy
for each layer:
	random init clusters
	while loss not converging:
		// assign spheres
		fix cluster centers, each time select child-cluster of minimum loss increment
		// powell optimize centers
		bounding leaf-spheres, Golden-section search
		
// cluster receivers
```

