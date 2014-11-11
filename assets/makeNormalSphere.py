with open('sphere.obj') as f:
    lines = f.readlines()

vertecies = filter(lambda line: line.startswith('v '), lines)
faces     = filter(lambda line: line.startswith('f '), lines)

normals = [ 'vn' + line[1:] for line in vertecies ]
newFaces = [
    (
        'f ' +
        ' '.join([
            '{0}//{0}'.format(index)
            for index in line.split()[1:]
        ]) +
        '\n'
    )
    for line in faces
]

with open('sphereWithNormals.obj', 'w') as f:
    lines = vertecies + normals + newFaces
    f.write(''.join(lines))
