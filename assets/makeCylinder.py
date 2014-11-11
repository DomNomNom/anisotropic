import numpy as np

tau = np.radians(360)

sides = 100

thetas = np.linspace(0.0, tau, num=sides, endpoint=False)
# print map(np.degrees, thetas)

def circle(y=0.0):
    return np.array([
        [np.cos(theta), y, np.sin(theta)]
        for theta in thetas
    ])



vertices = np.vstack([
    [0.0,  1.0, 0.0],      # 0 top center
    [0.0, -1.0, 0.0],      # 1 bot center
    circle( 1.0),   # 2 .. sides+2-1
    circle(-1.0),   #
])

normals = np.vstack([
    [0.0,  1.0, 0.0],       # 0            top
    [0.0, -1.0, 0.0],       # 1            bot
    circle(0.0),            # 2..sides+2   sides
])

def vertexIndexTop(thetaIndex):
    return thetaIndex+2

def vertexIndexBot(thetaIndex):
    return thetaIndex+2+sides

def normalIndex(thetaIndex):
    return thetaIndex+2

facesTop = []
facesBot = []
facesSides = []
for i in xrange(len(thetas)):
    n = (i+1) % len(thetas) # next index

    facesTop.append([
        [0,                 0],  # center
        [vertexIndexTop(i), 0],
        [vertexIndexTop(n), 0],
    ])
    facesBot.append([
        [1,                 1], # center
        [vertexIndexBot(n), 1],
        [vertexIndexBot(i), 1], # note: reversed last two
    ])

    facesSides.append([  # bottom left triangle
        [vertexIndexBot(i), normalIndex(i)],
        [vertexIndexBot(n), normalIndex(n)],
        [vertexIndexTop(i), normalIndex(i)],
    ])
    facesSides.append([  # top right triange
        [vertexIndexTop(n), normalIndex(n)],
        [vertexIndexTop(i), normalIndex(i)],
        [vertexIndexBot(n), normalIndex(n)],
    ])

faces = np.concatenate([
    facesTop,
    facesBot,
    facesSides
])

faces += 1  # .obj has 1-based indecies



# make the file string
vertices = np.round(vertices, decimals=2)
normals  = np.round(normals , decimals=2)

lines = []

for vert in vertices:
    lines.append('v ' + ' '.join(map(str, vert)))

lines.append('\n\n\n')

for normal in normals:
    lines.append('vn ' + ' '.join(map(str, normal)))

lines.append('\n\n\n')

for face in faces:
    lines.append('f ' + ' '.join([
        '{}//{}'.format(vertIndex, normalIndex)
        for vertIndex, normalIndex in face
    ]))

lines.append('')

# print '\n'.join(lines)
with open('cylinder{:03d}.obj'.format(sides), 'w') as f:
    f.write('\n'.join(lines))
