from path import path

d = path('./logs')

tim = {}
sol = {}

for i in [1, 3, 6]:
    for f in d.files("*.dom.log"+str(i)):
        log = open(f,'r')
        ist = f.split('/')[2].split(".col")[0]
        if ist not in tim:
            tim[ist] = []
            sol[ist] = []
        for line in log:
            l = line.strip().split(' ')
            if l[0] == "Nodes":
                if float(l[4]) > 300.0:
                    tim[ist].append( 300 )
                else:
                    tim[ist].append( l[4] )
            if l[0] == "X(G)":
                sol[ist].append( l[2] )

print "Istance S1 S2 S3"
for d,t in sorted(tim.iteritems()):
    if t:
        if len(t) == 3:
            print d, t[0], t[1], t[2]
